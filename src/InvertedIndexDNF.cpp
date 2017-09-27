//#include "utils/FileLoader.hpp"
#include "utils/RedisLoader.hpp"
#include "index/InversedIndex.hpp"

#include <set>
#include <iostream>
#include <json/json.h>
#include <json/value.h>

int main() {
    std::cout << "===== Reversed Index Service =====" << std::endl;
    clock_t t_start, t_end, t_query;
    t_start = clock();

    //boost::optional<FileLoader*> fl = FileLoader::GetInstance(DataFile, FieldFile);
    std::string ip = "127.0.0.1";
    int port = 6379;
    std::string db_index = "21";
    std::string ad_prefix = "what the fuck";
    int timeout = 30000;
    boost::optional < RedisLoader * > fl = RedisLoader::GetInstance(ip, port, db_index, ad_prefix, timeout);
    if (fl) {
        //std::map<std::string, std::vector<std::string>> data;
        Json::Value data;
        fl.get()->LoadRedis(data);

        // insert
        InversedIndex *ri = new InversedIndex();
        for (int i = 0; i < data.size(); i++) {
            //std::cout << data[i]["id"].asString() << std::endl;
            //std::cout << data[i]["conditions"].toStyledString() << std::endl;
            ri->Insert(data[i]);
        }
        // query
        Json::Value query;
        Json::Value result;
        Json::Value age;
        Json::Value area;
        Json::Value gender;
        Json::Value tag;
        Json::Value os;
        gender[0u] = "-1";
        age[0u] = "-1";
        //age[1] = "4";
        //age[2] = "5";
        //age[3] = "1";
        area[0u] = "2";
        area[1] = "1";
        //area[2] = "3";
        os[0u] = "-1";
        //tag[0u] = "14";
        //tag[1] = "14";
        query["audience_genders"] = gender;
        query["audience_agegroups"] = age;
        query["audience_areacategorys"] = area;
        query["audience_oss"] = os;
        //query["broadcaster_tags"] = tag;

        t_query = clock();
        ri->Select(query, result);

        t_end = clock();
        std::cout << "The whole time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << std::endl;
        std::cout << "The query time: " << (double) (t_end - t_query) / CLOCKS_PER_SEC << std::endl;

        //for(auto it:matchingDocs) std::cout << it << std::endl;
        //std::cout << "The number of matching docs: " << matchingDocs.size() << std::endl;
        //std::cout << std::endl << " result " << result["doc"] << std::endl;
        std::cout << std::endl << " result " << result << std::endl;
    }
    return 0;
}
