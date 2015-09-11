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
    if (i != NULL && i->has_volume) {
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

/* This function is only called from TUI::update_panels when a full sink inputs reload is required.
 Monitor streams could be "garbage-collected" here if corresponding sink input is not alive anymore
 However, it's not implemented. PA automatically destroys those streams but we still hold a pointer
 to them in detectorStreams. If PA assigns the already used before stream index to a new sink input,
 it won't be monitored. So this is a resource leak but I'm too lazy to check streams for existence.
 Maybe later.
*/

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
        get_object(userdata)->set_external_change(FULL);
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

void read_monitor_cb(pa_stream *p, size_t nbytes, void *userdata) {
    const void* data;
    size_t size;
    pa_stream_peek(p, &data, &size);
    bool sound = false;
    for (int i = 0; i < size; i++) {
        if (((uint8_t*) data)[i] != 128) {
            sound = true;
        }
    }
    uint32_t idx = pa_stream_get_monitor_stream(p);
    get_object(userdata)->set_sound_state(idx, sound);
    pa_stream_drop(p);
}

PSink PAWrapper::wrap_sink(PSinkInfo sink) {
    string sink_name = string(sink->name);
    PDetector detector = detectorStreams[sink->index];
    if (detector == nullptr) {
        pa_stream* detectorStream = pa_stream_new(context,
                "Sound presence detector", &sampleSpec, NULL);
        detector = PDetector(new Detector { detectorStream, false });
        detectorStreams[sink->index] = detector;
        pa_stream_set_monitor_stream(detectorStream, sink->index);
        pa_stream_set_read_callback(detectorStream, &read_monitor_cb, this);
        pa_stream_connect_record(detectorStream, NULL, NULL,
                PA_STREAM_PEAK_DETECT);
    }
    return PSink(
            new Sink { sink->index, sink->client, "", sink_name, "", sink->mute
                    == 1, sink->volume, detector->sound > 0 });
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

void PAWrapper::set_vol(uint32_t index, pa_cvolume* pvol) {
    pa_threaded_mainloop_lock(mainloop);
    wait(
            pa_context_set_sink_input_volume(context, index, pvol, success_cb,
                    this));
    pa_threaded_mainloop_unlock(mainloop);
}

PSink PAWrapper::change_volume(uint32_t index, int change, bool inc) {
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

void PAWrapper::set_external_change(change_type type) {
    external_change = type;
}

void PAWrapper::set_client_name(const char* name) {
    client_name = string(name);
}

change_type PAWrapper::get_external_change() {
    change_type result = external_change;
    external_change = NONE;
    return result;
}

PSink PAWrapper::set_volume(uint32_t index, int vol) {
    pa_cvolume* pvol = &sinks[index]->volume;
    set_vol(index, pa_cvolume_set(pvol, pvol->channels, vol));
    return sinks[index];
}

PSink PAWrapper::toggle_mute(uint32_t index) {
    pa_context_set_sink_input_mute(context, index, !sinks[index]->muted, NULL,
    NULL);
    refresh_sink(index);
    return sinks[index];
}

void PAWrapper::set_sound_state(uint32_t idx, bool sound) {
    PDetector detector = detectorStreams[idx];
    if (detector == nullptr) {
        return;
    }
    int8_t oldSound = detector->sound;
    detector->sound += sound ? 1 : -1;
    if (detector->sound > 10) {
        detector->sound = 10;
    }
    if (detector->sound < 0) {
        detector->sound = 0;
    }
    PSink sink = sinks[idx];
    if (sink != nullptr) {
        sink->sound = detector->sound > 0;
        if ((bool) oldSound != sound) {
            set_external_change(REDRAW);
        }
    }
}
