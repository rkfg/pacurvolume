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
        get_object(userdata)->add_sink(*i);
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

void PAWrapper::add_sink(pa_sink_input_info sink) {
    sinks[sink.index] = sink;
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

PAWrapper::PAWrapper(const char* client_name) {
    mainloop = pa_threaded_mainloop_new();
    pa_threaded_mainloop_start(mainloop);
    pa_threaded_mainloop_lock(mainloop);
    context = pa_context_new(pa_threaded_mainloop_get_api(mainloop),
            client_name);
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

void PAWrapper::print_sinks() {
    for (pair<uint32_t, pa_sink_input_info> sink : sinks) {
        cout << "Sink name: " << sink.second.name << " volume: "
                << sink.second.volume.values[0];
        delimiter();
    }
}

PAWrapper::~PAWrapper() {
    pa_threaded_mainloop_free(mainloop);
}

unsigned int PAWrapper::get_sinks_count() {
    return sinks.size();
}

vector<Sink> PAWrapper::list_sinks() {
    vector<Sink> result;
    for (pair<uint32_t, pa_sink_input_info> sink : sinks) {
        result.push_back(wrap_sink(sink.second));
    }
    return result;
}

void client_info_cb(pa_context *c, const pa_client_info*i, int eol,
        void *userdata) {
    if (i != NULL) {
        get_object(userdata)->set_client_name(i->name);
    } else {
        complete(userdata);
    }
}

Sink PAWrapper::wrap_sink(pa_sink_input_info sink) {
    string sink_name = string(sink.name);
    pa_threaded_mainloop_lock(mainloop);
    wait(
            pa_context_get_client_info(context, sink.client, client_info_cb,
                    this));
    pa_threaded_mainloop_unlock(mainloop);
    return Sink { sink.index, client_name + (!sink_name.empty() ? ": " + sink_name : ""),
            sink.volume.values[0] };
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

Sink PAWrapper::change_volume(unsigned int index, int change, bool inc) {
    pa_sink_input_info sink = sinks[index];
    pa_cvolume* pvol;
    if (inc) {
        pvol = pa_cvolume_inc_clamp(&sink.volume, change, PA_VOLUME_UI_MAX);
    } else {
        pvol = pa_cvolume_dec(&sink.volume, change);
    }
    pa_threaded_mainloop_lock(mainloop);
    wait(
            pa_context_set_sink_input_volume(context, index, pvol, success_cb,
                    this));
    pa_threaded_mainloop_unlock(mainloop);
    refresh_sink(index);
    return wrap_sink(sinks[index]);
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
