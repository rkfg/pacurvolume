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
    string name;
    pa_volume_t volume;
};

typedef shared_ptr<Sink> PSink;

class PAWrapper {
private:
    pa_threaded_mainloop* mainloop;
    pa_context* context;
    map<uint32_t, pa_sink_input_info> sinks;
    PSink wrap_sink(pa_sink_input_info sink);
    string client_name;
    friend void complete(void* userdata);
    void wait(pa_operation* o, bool debug = false);
    volatile bool external_change = false;
public:
    PAWrapper(const char* client_name = "myapp");
    virtual ~PAWrapper();
    friend void sink_input_info_list_cb(pa_context *c,
            const pa_sink_input_info *i, int eol, void *userdata);
    void print_sinks();
    void collect_sinks();
    void add_sink(const pa_sink_input_info sink);
    void refresh_sink(uint32_t idx);
    unsigned int get_sinks_count();
    shared_ptr<vector<PSink>> list_sinks();
    PSink change_volume(unsigned int index, int change, bool inc);
    void set_client_name(const char* name);
    void set_external_change();
    bool get_external_change();
};

#endif /* SRC_PAWRAPPER_H_ */
