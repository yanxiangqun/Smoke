//
// Created by yanxq on 2017/2/22.
//

#include "smoke_appender.h"
#include "smoke_utils/file_util.h"
#include "smoke_jni_log.h"
#include "mmap_file.h"
#include "log_buffer.h"
#include "smoke_utils/time_utils.h"
#include "bootrun.h"
#include <assert.h>
#include <mutex>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <unistd.h>
#include <inttypes.h>

#define LOG_FILE_SUFFIX "sf"

extern void log_format(const smoke::SmokeLog *_info, const char* _log_body, PtrBuffer& _log);

static volatile bool sg_log_closed = true;
static std::string sg_cache_dir;
static std::string sg_log_dir;
static FILE *sg_log_file;
static TAppenderMode sg_mode = TAppenderMode::appenderAsync;
static time_t sg_open_file_time;
static std::string sg_name_prefix;
static std::string sg_current_dir;
static std::recursive_mutex sg_mutex_log_file;
static std::mutex sg_mutex_buffer_async;
static std::condition_variable sg_condition_buffer_async;

static std::thread *sg_thread_async = NULL;
static void *sg_mmap_ptr = NULL;
static LogBuffer *sg_log__buffer;

static const unsigned int BUFFER_BLOCK_LENGTH = 80 * getpagesize();

static void __write_tips_to_console(const char *_tips_format, ...) {
    if (NULL == _tips_format) {
        return;
    }

    va_list ap;
    va_start(ap, _tips_format);
    smoke_jni::console_info(__FUNCTION__,_tips_format,ap);
    va_end(ap);
}

static bool __write_file(const void *_data, size_t _len, FILE *_file) {
    if (NULL == _file) {
        assert(false);
        return false;
    }

    long before_len = ftell(_file);
    if (before_len < 0) return false;

    if (1 != fwrite(_data, _len, 1, _file)) {
        int err = ferror(_file);

        __write_tips_to_console("write file error:%d", err);

        ftruncate(fileno(_file), before_len);
        fseek(_file, 0, SEEK_END);

        char err_log[256] = {0};
        snprintf(err_log, sizeof(err_log), "\nwrite file error:%d\n", err);

        char tmp[256] = {0};
        size_t len = sizeof(tmp);
        LogBuffer::Write(err_log, strnlen(err_log, sizeof(err_log)), tmp, len);

        fwrite(tmp, len, 1, _file);
        return false;
    }

    return true;
}

static void __get_log_file_name(const timeval &_tv, const std::string &_log_dir, const char *_prefix,
                                const std::string &_file_suffix, char *_filepath, unsigned int _len) {
    time_t sec = _tv.tv_sec;
    tm t_cur = *localtime((const time_t*)&sec);

    std::string log_file_path = _log_dir;
    log_file_path += "/";
    log_file_path += _prefix;
    char temp [64] = {0};
    snprintf(temp, 64, "_%d%02d%02d", 1900 + t_cur.tm_year, 1 + t_cur.tm_mon, t_cur.tm_mday);
    log_file_path += temp;
    log_file_path += ".";
    log_file_path += _file_suffix;
    strncpy(_filepath, log_file_path.c_str(), _len - 1);
    _filepath[_len - 1] = '\0';
}

static bool __open_log_file(const std::string& _log_dir) {
    //todo 这里要做判断，如果 _lod_dir 不可用，启用 cache_dir
    if (sg_log_dir.empty()) {
        return false;
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);

    if (NULL != sg_log_file) {
        time_t sec = tv.tv_sec;
        tm tcur = *localtime((const time_t*)&sec);
        tm filetm = *localtime(&sg_open_file_time);

        if (filetm.tm_year == tcur.tm_year
            && filetm.tm_mon == tcur.tm_mon
            && filetm.tm_mday == tcur.tm_mday
            && sg_current_dir == _log_dir) {
            return true;
        }

        fclose(sg_log_file);
        sg_log_file = NULL;
    }

    static time_t s_last_time = 0;
    static uint64_t s_last_tick = 0;
    static char s_last_file_path[1024] = {0};

    uint64_t now_tick = gettickcount();
    time_t now_time = tv.tv_sec;

    sg_open_file_time = tv.tv_sec;
    sg_current_dir = _log_dir;

    char log_file_path[1024] = {0};
    __get_log_file_name(tv, _log_dir, sg_name_prefix.c_str(), LOG_FILE_SUFFIX, log_file_path , 1024);

    if (now_time < s_last_time) {//todo why?
        sg_log_file = fopen(s_last_file_path, "ab");

        if (NULL == sg_log_file) {
            __write_tips_to_console("open file error:%d %s, path:%s", errno, strerror(errno),
                                    s_last_file_path);
        }

#ifdef __APPLE__
        assert(sg_log_file);
#endif
        return NULL != sg_log_file;
    }

    sg_log_file = fopen(log_file_path, "ab");

    if (NULL == sg_log_file) {
        __write_tips_to_console("open file error:%d %s, path:%s", errno, strerror(errno),
                                log_file_path);
    }

    time_t tick_distance = (now_tick - s_last_tick) / 1000 + 300;
    if (0 != s_last_time && (now_time - s_last_time) > tick_distance) {
        struct tm tm_tmp = *localtime((const time_t*)&s_last_time);
        char last_time_str[64] = {0};
        strftime(last_time_str, sizeof(last_time_str), "%Y-%m-%d %z %H:%M:%S", &tm_tmp);

        tm_tmp = *localtime((const time_t*)&now_time);
        char now_time_str[64] = {0};
        strftime(now_time_str, sizeof(now_time_str), "%Y-%m-%d %z %H:%M:%S", &tm_tmp);

        char log[1024] = {0};
//        snprintf(log, sizeof(log), "[F][ last log file:%s from %s to %s, time_diff:%ld, tick_diff:%" PRIu64 "\n", s_last_file_path, last_time_str, now_time_str, now_time-s_last_time, now_tick-s_last_tick);
        char tmp[2 * 1024] = {0};
        size_t len = sizeof(tmp);
        LogBuffer::Write(log, strnlen(log, sizeof(log)), tmp, len);
        __write_file(tmp, len, sg_log_file);
    }

    memcpy(s_last_file_path, log_file_path, sizeof(s_last_file_path));
    s_last_tick = now_tick;
    s_last_time = now_time;

#ifdef __APPLE__
    assert(sg_log_file);
#endif
    return NULL != sg_log_file;
}

static void __close_log_file() {
    if (NULL == sg_log_file) {
        return;
    }

    sg_open_file_time = 0;
    fclose(sg_log_file);
    sg_log_file = NULL;
}

static void __log_to_file(const void* _data, size_t _len) {
    if (NULL == _data || 0 == _len || sg_log_dir.empty()) {
        return;
    }

    std::unique_lock<std::recursive_mutex> mutex_log_file(sg_mutex_log_file);

    if (__open_log_file(sg_log_dir)) {
        __write_file(_data,_len,sg_log_file);
        if (appenderAsync == sg_mode) {
            //todo 立马关闭，然后定时开启写？
            __close_log_file();
        }
    }
}

static void __write_tips_to_file(const char *_tips_fmt, ...) {
    if (_tips_fmt == NULL) {
        return;
    }

    char tips_info[4096] = {0};
    va_list ap;
    va_start(ap, _tips_fmt);
    vsnprintf(tips_info, sizeof(tips_info), _tips_fmt, ap);
    va_end(ap);

    char tmp[8 * 1024] = {0};
    size_t len = sizeof(tmp);

    LogBuffer::Write(tips_info, strnlen(tips_info, sizeof(tips_info)), tmp, len);

    __log_to_file(tmp, len);
}

static void __del_timeout_file(const char * _dir) {
    if (!fileUtil::exists(_dir)) {
        return;
    }

    time_t now_time = time(NULL);

    //todo 删除过期文件；yanxq



}

static void __append_sync(smoke::SmokeLog &_log) {
    char temp[16 * 1024] = {0};     // tell perry,ray if you want modify size.
    PtrBuffer log(temp, 0, sizeof(temp));
    log_format(&_log, NULL, log);

    char buffer_crypt[16 * 1024] = {0};
    size_t len = 16 * 1024;
    if (!LogBuffer::Write(log.Ptr(), log.Length(), buffer_crypt, len)) {
        return;
    }

    __log_to_file(buffer_crypt, len);
}

static void __async_log_thread() {
    while (true) {
        std::unique_lock<std::mutex> lock_buffer(sg_mutex_buffer_async);

        if (NULL == sg_log__buffer) {
            smoke_jni::console_debug(__FUNCTION__,"Log buffer is NULL!");
            break;
        }

        smoke_jni::console_debug(__FUNCTION__,"It is time to flush buffer log.");

        AutoBuffer tmp;
        sg_log__buffer->Flush(tmp);
        sg_mutex_buffer_async.unlock();

        if (NULL != tmp.Ptr()) {
            __log_to_file(tmp.Ptr(), tmp.Length());
        }

        if (sg_log_closed) {
            break;
        }

        sg_condition_buffer_async.wait_for(lock_buffer,std::chrono::minutes(1));
    }
}

static void __append_async(smoke::SmokeLog &_log) {
    std::unique_lock<std::mutex> buffer_lock(sg_mutex_buffer_async);
    if (sg_log__buffer == NULL) {
        smoke_jni::console_debug(__FUNCTION__,"sg_log_buffer is NULL!");
        return;
    }

    char temp[16*1024] = {0};       //tell perry,ray if you want modify size.
    PtrBuffer log_buff(temp, 0, sizeof(temp));
    log_format(&_log,NULL,log_buff);

    if (sg_log__buffer->GetData().Length() >= BUFFER_BLOCK_LENGTH * 4/5) {
        int ret = snprintf(temp, sizeof(temp), "[F][ sg_buffer_async.Length() >= BUFFER_BLOCK_LENTH*4/5, len: %d\n", (int)sg_log__buffer->GetData().Length());
        log_buff.Length(ret, ret);
    }

    if (!sg_log__buffer->Write(log_buff.Ptr(), (unsigned int)log_buff.Length())) {
        return;
    }

    if (sg_log__buffer->GetData().Length() >= BUFFER_BLOCK_LENGTH *1/3) {
        sg_condition_buffer_async.notify_all();
    }
}

void append_log(smoke::SmokeLog &_log) {
    if (sg_log_closed) {
        smoke_jni::console_warn(__FUNCTION__,"Write log when file closed!");
        return;
    }

    if (appenderSync == sg_mode) {
        __append_sync(_log);
    } else {
        __append_async(_log);
    }
}


static void get_mark_info(char *_info, size_t _info_length) {
    struct timeval tv;
    gettimeofday(&tv, 0);
    time_t sec = tv.tv_sec;
    struct tm tm_tmp = *localtime((const time_t*)&sec);
    char tmp_time[64] = {0};
    strftime(tmp_time, sizeof(tmp_time), "%Y-%m-%d %z %H:%M:%S", &tm_tmp);
    snprintf(_info, _info_length, "[%s]", tmp_time);
}

void appender_open(TAppenderMode _mode, const char* _dir, const char *_cache_dir, const char* _name_prefix) {
    assert(_dir);
    assert(_name_prefix);

    if (!sg_log_closed) {
        __write_tips_to_file("Smoke appender has already been opened. dir: %s name_prefix: %s",_dir,_name_prefix);
        return;
    }

    fileUtil::create_dirs(_dir);
    __del_timeout_file(_dir);

    char mmap_file_path[512] = {0};
    snprintf(mmap_file_path, sizeof(mmap_file_path),"%s/%s.mmap2",_dir,_name_prefix);

    bool is_using_mmap = false;
    sg_mmap_ptr = open_mmap(mmap_file_path,BUFFER_BLOCK_LENGTH);
    if ((is_using_mmap = (sg_mmap_ptr != NULL))) {
        //TODO 先关闭 compress
        sg_log__buffer = new LogBuffer(sg_mmap_ptr,BUFFER_BLOCK_LENGTH, false);
    } else {
        char * buffer = new char[BUFFER_BLOCK_LENGTH];
        sg_log__buffer = new LogBuffer(buffer,BUFFER_BLOCK_LENGTH,false);
    }

    smoke_jni::console_debug(__FUNCTION__,"Smoke is using mmap : [%s]",is_using_mmap);

    if (sg_log__buffer->GetData().Ptr() == NULL) {
        if (is_using_mmap) {
            close_mmap(sg_mmap_ptr);
        }
        return;
    }

    AutoBuffer autoBuffer;
    sg_log__buffer->Flush(autoBuffer);

    std::unique_lock<std::recursive_mutex> mutex_log_file(sg_mutex_log_file);
    sg_log_dir = std::string(_dir);
    sg_cache_dir = std::string(_cache_dir);
    sg_name_prefix = std::string(_name_prefix);
    sg_log_closed = false;
    appender_set_mode(_mode);
    mutex_log_file.unlock();

    //todo yanxq
    char mark_info[512] = {0};
    get_mark_info(mark_info, sizeof(mark_info));

    if (autoBuffer.Ptr() != NULL) {
        //写上次 mmap 的内容？
        __write_tips_to_file("~~~~~ begin of mmap ~~~~~\n");
        __log_to_file(autoBuffer.Ptr(), autoBuffer.Length());
        __write_tips_to_file("~~~~~ end of mmap ~~~~~%s\n", mark_info);
    }

    char appender_info[728] = {0};
    snprintf(appender_info, sizeof(appender_info), "======== %s ========\n", mark_info);

    __write_tips_to_file(appender_info);

    BOOT_RUN_EXIT(appender_close);
}


void appender_open_with_cache(TAppenderMode _mode, const std::string& _cache_dir, const std::string& _log_dir, const char* _name_prefix) {

}

void appender_flush() {
    sg_condition_buffer_async.notify_all();
}

void appender_flush_sync() {
    if (appenderSync == sg_mode) {
        return;
    }

    std::unique_lock<std::mutex> lock_buffer(sg_mutex_buffer_async);

    if (NULL == sg_log__buffer) {
        smoke_jni::console_debug(__FUNCTION__,"log buffer is NULL!");
        return;
    }

    AutoBuffer buffer;
    sg_log__buffer->Flush(buffer);

    lock_buffer.unlock();

    if (buffer.Ptr()) {
        __log_to_file(buffer.Ptr(),buffer.Length());
    }
}

void appender_close() {
    if (sg_log_closed) {
        return;
    }

    smoke_jni::console_debug(__FUNCTION__,"Smoke Closing...");

    sg_log_closed = true;
    sg_condition_buffer_async.notify_all();

    if (sg_thread_async != NULL) {
        sg_thread_async->join();
    }

    //初步 close
    std::unique_lock<std::mutex> lock_buffer(sg_mutex_buffer_async);
    if (sg_mmap_ptr != NULL) {
        close_mmap(sg_mmap_ptr);
    } else {
        // 内存缓存
        delete[] (char *) ((sg_log__buffer->GetData()).Ptr());
    }

    delete sg_log__buffer;
    sg_log__buffer = NULL;

    lock_buffer.unlock();

    std::unique_lock<std::recursive_mutex> log_file_lock(sg_mutex_log_file);
    __close_log_file();
}


void appender_set_mode(TAppenderMode _mode) {
    sg_mode = _mode;

    sg_condition_buffer_async.notify_all();

    if (_mode == appenderAsync && sg_thread_async == NULL) {
        sg_thread_async = new std::thread(__async_log_thread);
    }
}

std::string appender_get_filepath_from_timespan(int _timespan, const char *_prefix,
                                         std::vector<std::string> &_filepath_vec) {
    return std::string("");
}

std::string appender_get_current_log_path(char* _log_path, unsigned int _len) {
    return sg_current_dir;
}

std::string appender_get_current_log_cache_path(char* _logPath, unsigned int _len) {
    return sg_cache_dir;
}