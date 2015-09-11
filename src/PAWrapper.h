/*
 * PAWrapper.h
 *
 *  Created on: 26 Jul 2015
 *      Author: rkfg
 */

#ifndef SRC_PAWRAPPER_H_
#define SRC_PAWRAPPER_H_

#include <pulse/def.h>

#include <iostream>
#include <pulse/thread-mainloop.h>
#include <pulse/context.h>
#include <pulse/introspect.h>
#include <pulse/subscribe.h>
#include <pulse/stream.h>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <assert.h>
#include <memory>

using namespace std;

struct Sink {
    uint32_t idx;
    uint32_t client_idx;
    string name;
    string sink_name;
    string client_name;
    bool muted;
    pa_cvolume volume;
    bool sound;
};

struct Detector {
    pa_stream* stream;
    int8_t sound;
};

typedef shared_ptr<Sink> PSink;
typedef shared_ptr<pa_sink_input_info> PSinkInfo;
typedef shared_ptr<Detector> PDetector;

enum change_type {
    NONE, FULL, REDRAW
};

class PAWrapper {
private:
    pa_sample_spec sampleSpec { PA_SAMPLE_U8, 8000, 1 };
    pa_threaded_mainloop* mainloop;
    pa_context* context;
    map<uint32_t, PSink> sinks;
    map<uint32_t, PDetector> detectorStreams;
    PSink wrap_sink(PSinkInfo sink);
    string client_name;
    string app_name;
    friend void complete(void* userdata);
    void wait(pa_operation* o, bool debug = false);
    void set_vol(uint32_t index, pa_cvolume* pvol);
    volatile change_type external_change = NONE;
public:
    PAWrapper(string app_name = "myapp");
    virtual ~PAWrapper();
    friend void sink_input_info_list_cb(pa_context *c,
            const pa_sink_input_info *i, int eol, void *userdata);
    void collect_sinks();
    void add_sink(const PSinkInfo sink);
    void refresh_sink(uint32_t idx);
    unsigned int get_sinks_count();
    shared_ptr<vector<PSink>> list_sinks();
    PSink change_volume(uint32_t index, int change, bool inc);
    PSink set_volume(uint32_t index, int vol);
    PSink toggle_mute(uint32_t index);
    void set_client_name(const char* name);
    void set_external_change(change_type type);
    change_type get_external_change();
    void set_sound_state(uint32_t idx, bool sound);
};

#endif /* SRC_PAWRAPPER_H_ */
