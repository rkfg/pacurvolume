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
#include <vector>
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
};

typedef shared_ptr<Sink> PSink;
typedef shared_ptr<pa_sink_input_info> PSinkInfo;

class PAWrapper {
private:
    pa_threaded_mainloop* mainloop;
    pa_context* context;
    map<uint32_t, PSink> sinks;
    PSink wrap_sink(PSinkInfo sink);
    string client_name;
    string app_name;
    friend void complete(void* userdata);
    void wait(pa_operation* o, bool debug = false);
    void set_vol(unsigned int index, pa_cvolume* pvol);
    volatile bool external_change = false;
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
    PSink change_volume(unsigned int index, int change, bool inc);
    PSink set_volume(unsigned int index, int vol);
    void set_client_name(const char* name);
    void set_external_change();
    bool get_external_change();
};

#endif /* SRC_PAWRAPPER_H_ */
