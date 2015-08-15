/*
 * PAWrapper.cpp
 *
 *  Created on: 26 Jul 2015
 *      Author: rkfg
 */

#include "PAWrapper.h"

#include <pulse/operation.h>
#include <pulse/volume.h>

PAWrapper* get_object(void* userdata) {
    return static_cast<PAWrapper*>(userdata);
}

void complete(void* userdata) {
    pa_threaded_mainloop_signal(get_object(userdata)->mainloop, 0);
}

void delimiter() {
    cout << endl << string(20, '=') << endl;
}

void success_cb(pa_context *c, int success, void *userdata) {
    complete(userdata);
}

void server_info_cb(pa_context* c, const pa_server_info* i, void *userdata) {
    cout << "Server name: " << i->host_name << endl << "Version: "
            << i->server_version;
    delimiter();
    complete(userdata);
}

void sink_input_info_list_cb(pa_context *c, const pa_sink_input_info *i,
        int eol, void *userdata) {
    if (i != NULL) {
        get_object(userdata)->add_sink(PSinkInfo(new pa_sink_input_info(*i)));
    } else {
        complete(userdata);
    }
}

void connect_callback(pa_context* context, void* userdata) {
    if (pa_context_get_state(context) == PA_CONTEXT_READY) {
        cout << "Connected.";
        delimiter();
        complete(userdata);
    }
}

void PAWrapper::add_sink(PSinkInfo sink) {
    sinks[sink->index] = wrap_sink(sink);
}

void PAWrapper::collect_sinks() {
    pa_threaded_mainloop_lock(mainloop);
    sinks.clear();
    wait(
            pa_context_get_sink_input_info_list(context,
                    sink_input_info_list_cb, this));
    pa_threaded_mainloop_unlock(mainloop);
}

void event_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t idx,
        void *userdata) {
    if ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK)
            == PA_SUBSCRIPTION_EVENT_SINK_INPUT) {
        get_object(userdata)->set_external_change();
    }
}

PAWrapper::PAWrapper(string app_name) {
    this->app_name = app_name;
    mainloop = pa_threaded_mainloop_new();
    pa_threaded_mainloop_start(mainloop);
    pa_threaded_mainloop_lock(mainloop);
    context = pa_context_new(pa_threaded_mainloop_get_api(mainloop),
            this->app_name.c_str());
    pa_context_set_state_callback(context, connect_callback, this);
    pa_context_set_subscribe_callback(context, event_cb, this);
    pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS,
    NULL);
    pa_threaded_mainloop_wait(mainloop);
    wait(pa_context_get_server_info(context, server_info_cb, this));
    pa_operation_unref(
            pa_context_subscribe(context, PA_SUBSCRIPTION_MASK_SINK_INPUT, NULL,
            NULL));
    pa_threaded_mainloop_unlock(mainloop);
}

PAWrapper::~PAWrapper() {
    pa_context_disconnect(context);
    pa_threaded_mainloop_stop(mainloop);
    pa_threaded_mainloop_free(mainloop);
}

unsigned int PAWrapper::get_sinks_count() {
    return sinks.size();
}

void client_info_cb(pa_context *c, const pa_client_info*i, int eol,
        void *userdata) {
    if (i != NULL) {
        get_object(userdata)->set_client_name(i->name);
    } else {
        complete(userdata);
    }
}

shared_ptr<vector<PSink>> PAWrapper::list_sinks() {
    shared_ptr<vector<PSink>> result = shared_ptr<vector<PSink>>(
            new vector<PSink>);
    pa_threaded_mainloop_lock(mainloop);
    for (pair<uint32_t, PSink> sink_pair : sinks) {
        PSink sink = sink_pair.second;
        if (sink->name.empty()) {
            wait(
                    pa_context_get_client_info(context, sink->client_idx,
                            client_info_cb, this));
            sink->client_name = client_name;
            sink->name = client_name
                    + (!sink->sink_name.empty() ? ": " + sink->sink_name : "");
        }
        result->push_back(sink);
    }
    pa_threaded_mainloop_unlock(mainloop);
    return result;
}

PSink PAWrapper::wrap_sink(PSinkInfo sink) {
    string sink_name = string(sink->name);
    return PSink(
            new Sink { sink->index, sink->client, "", sink_name, "",
                    sink->volume });
}

void PAWrapper::wait(pa_operation* o, bool debug) {
    assert(o);
    while (pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mainloop);
    }
    pa_operation_unref(o);
}

void PAWrapper::refresh_sink(uint32_t idx) {
    pa_threaded_mainloop_lock(mainloop);
    wait(
            pa_context_get_sink_input_info(context, idx,
                    sink_input_info_list_cb, this), true);
    pa_threaded_mainloop_unlock(mainloop);
}

void PAWrapper::set_vol(unsigned int index, pa_cvolume* pvol) {
    pa_threaded_mainloop_lock(mainloop);
    wait(
            pa_context_set_sink_input_volume(context, index, pvol, success_cb,
                    this));
    pa_threaded_mainloop_unlock(mainloop);
    refresh_sink(index);
}

PSink PAWrapper::change_volume(unsigned int index, int change, bool inc) {
    PSink sink = sinks[index];
    pa_cvolume* pvol;
    if (inc) {
        pvol = pa_cvolume_inc_clamp(&sink->volume, change, PA_VOLUME_UI_MAX);
    } else {
        pvol = pa_cvolume_dec(&sink->volume, change);
    }
    set_vol(index, pvol);
    return sinks[index];
}

void PAWrapper::set_external_change() {
    external_change = true;
}

void PAWrapper::set_client_name(const char* name) {
    client_name = string(name);
}

bool PAWrapper::get_external_change() {
    bool result = external_change;
    external_change = false;
    return result;
}

PSink PAWrapper::set_volume(unsigned int index, int vol) {
    pa_cvolume* pvol = &sinks[index]->volume;
    set_vol(index, pa_cvolume_set(pvol, pvol->channels, vol));
    return sinks[index];
}
