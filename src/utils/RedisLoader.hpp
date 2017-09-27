/*
 * Created by brigbaby on 9/7/17.
 */
#ifndef REVERSEDINDEXSERVICE_REDISLOADER_HPP
#define REVERSEDINDEXSERVICE_REDISLOADER_HPP

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_set>

#include <hiredis/hiredis.h>
#include <time.h>
#include <json/json.h>
#include <json/value.h>

class RedisLoader{

private:
    std::string IP="";
    int Port;
    std::string DB_index;
    std::string AD_prefix;
    int Timeout;

    redisReply* GetADs(std::string ip, int port, std::string db_index, std::string ad_prefix, redisContext** ptr_ptr_context);
    bool FailToReply(redisReply** ptr_prt_reply_of_cmd, const char *cmd);
    void GetErrorPosition(std::string filePosition, int linePosition);
    RedisLoader(std::string ip, int port, std::string db_index, std::string ad_prefix, int timeout):
            IP(ip), Port(port), DB_index(db_index), AD_prefix(ad_prefix), Timeout(timeout){}

public:
    //RedisLoader(std::string ip, int port, std::string db_index, std::string ad_prefix, int timeout):
    //        IP(ip), Port(port), DB_index(db_index), AD_prefix(ad_prefix), Timeout(timeout){}

    static boost::optional<RedisLoader*> GetInstance(std::string IP, int Port, std::string DB_index, std::string AD_prefix, int Timeout){
        if(IP.empty()){
            return boost::none;
        }
        else {
            return new RedisLoader(IP, Port, DB_index, AD_prefix, Timeout);
        }
    }

    void LoadRedis(Json::Value& result);
};

void RedisLoader::LoadRedis(Json::Value &result) {
    std::unordered_set<std::string> header{"audience_gender", "audience_agegroups", "audience_areacategorys", "audience_oss", "broadcaster_tags"};

    struct timeval redis_timeout;
    redis_timeout.tv_sec = 0;
    redis_timeout.tv_usec = Timeout;

    redisContext* context = NULL;
    context = redisConnectWithTimeout(IP.data(), Port, redis_timeout);
    if(context == NULL || context>err){
        std::cout << "Connect to redisServer failed" << std::endl;
        GetErrorPosition(__FILE__, __LINE__);
        return;
    }
    if(redisSetTimeout(context, redis_timeout) != REDIS_OK)
    {
        std::cout << "Connect to redisServer time out" << std::endl;
        GetErrorPosition(__FILE__, __LINE__);
        return;
    }

    redisReply *reply_of_ADs = GetADs(IP, Port, DB_index, AD_prefix, &context);
    if (reply_of_ADs == NULL) {
        std::cout << "can not get ads data" << std::endl;
        return;
    }

    //load each AD
    for (int index_ad = 0; index_ad < (reply_of_ADs->elements); ++index_ad) {
        Json::Value doc;
        Json::Value conditions;
        Json::Value condition;
        Json::Value FieldList;
        std::string ad_name = reply_of_ADs->element[index_ad]->str;

        std::string hgetall ="hgetall "+ad_name;
        redisReply *reply_name_and_value = (redisReply *) redisCommand(context, hgetall.c_str());
        bool isNULL = FailToReply(&reply_name_and_value, hgetall.c_str());
        if(isNULL) {
            GetErrorPosition(__FILE__, __LINE__);
            return;
        }
        if(reply_name_and_value->elements %2 != 0){
            std::cout << "the value of some keys in this ad //" << ad_name << "// is missing" << std::endl;
            GetErrorPosition(__FILE__, __LINE__);
            return;
        }

        for (int index_n_v = 0; index_n_v < (reply_name_and_value->elements) / 2; ++index_n_v) {
            std::string attribute_name = reply_name_and_value->element[index_n_v*2]->str;
            std::string attribute_value = reply_name_and_value->element[index_n_v*2 + 1]->str;

            if (header.find(attribute_name) != header.end()) {
                std::vector <std::string> values;
                boost::split(values, attribute_value, boost::is_any_of(","));
                int valueIndex = 0;
                if (values.size() == 0) {}
                else {
                    for (auto &it : values) {
                        // add field value
                        FieldList[valueIndex++] = it;
                    }
                }
                if (FieldList.isNull() == false) {
                    condition[attribute_name] = FieldList;
                }
            }
        }
        conditions.append(condition);
        doc["id"] = ad_name;
        doc["conditions"] = conditions;
        result.append(doc);
    }
}

redisReply* RedisLoader::GetADs(std::string ip, int port, std::string db_index, std::string ad_prefix, redisContext** ptr_ptr_context){
    const char* IP = ip.data();
    redisContext* context = redisConnect(IP, port);
    if(context->err) {
        std::cout << "connect redisServer err: " << context->errstr << std::endl;
        return NULL;
    }
    std::cout << "connect redisServer success" << std::endl;

    //select DB
    std::string get_DB_cmd = "select " + db_index;
    redisReply *reply_of_DB = (redisReply *)redisCommand(context, get_DB_cmd.c_str());
    bool isNULL = FailToReply(&reply_of_DB, get_DB_cmd.c_str());
    if(isNULL) {
        GetErrorPosition(__FILE__, __LINE__);
        return NULL;
    }
    if(!(reply_of_DB->type == REDIS_REPLY_STATUS && reply_of_DB->str != "OK")){
        std::cout << "command execute failure: " << get_DB_cmd << std::endl;
        GetErrorPosition(__FILE__, __LINE__);
        freeReplyObject(reply_of_DB);
        return NULL;
    }
    std::cout << "command execute success: " << get_DB_cmd << std::endl;
    freeReplyObject(reply_of_DB);

    //keys SARA_KEY_AD_BASEDATA*
    std::string get_ADs_cmd = "keys "+ad_prefix;
    redisReply *reply_of_ADs = (redisReply *)redisCommand(context, get_ADs_cmd.c_str());
    isNULL = FailToReply(&reply_of_ADs, get_ADs_cmd.c_str());
    if(isNULL) {
        GetErrorPosition(__FILE__, __LINE__);
        return NULL;
    }
    if((reply_of_ADs->type != REDIS_REPLY_ARRAY) || (reply_of_ADs->elements == 0)){
        std::cout << "no ads in current DB, wrong DB or wrong key words" << std::endl;
        GetErrorPosition(__FILE__, __LINE__);
        return NULL;
    }
    std::cout << "command execute success: " << get_ADs_cmd << std::endl;
    *ptr_ptr_context = context;
    return reply_of_ADs;
}

bool RedisLoader::FailToReply(redisReply** ptr_prt_reply_of_cmd, const char *cmd) {
    redisReply* reply_of_cmd = *ptr_prt_reply_of_cmd;
    if(reply_of_cmd == NULL){
        std::cout << "command execute failure: " << cmd << std::endl;
        return true;
    }
    return false;
}

void RedisLoader::GetErrorPosition(std::string filePosition, int linePosition) {
    std::cout << filePosition << " : " << linePosition << std::endl;
}

#endif //REVERSEDINDEXSERVICE_REDISLOADER_HPP
