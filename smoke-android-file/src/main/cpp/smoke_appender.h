//
// Created by yanxq on 2017/2/22.
//

#include <string>
#include <vector>
#include "smoke_base.h"

#ifndef SMOKE_SMOKE_APPENDER_H
#define SMOKE_SMOKE_APPENDER_H

enum AppenderMode {
    MODE_ASYNC = 0,
    MODE_SYNC,
};

void append_log(smoke::SmokeLog &_log);
void appender_open(AppenderMode _mode, const char* _dir, const char *_cache_dir, const char* _name_prefix);
void appender_flush();
void appender_flush_sync();
void appender_close();
void appender_set_mode(AppenderMode _mode);
std::string appender_get_filepath_from_timespan(int _timespan, const char *_prefix,
                                         std::vector<std::string> &_filepath_vec);
std::string appender_get_current_log_path(char* _log_path, unsigned int _len);
std::string appender_get_current_log_cache_path(char* _logPath, unsigned int _len);

#endif //SMOKE_SMOKE_APPENDER_H
