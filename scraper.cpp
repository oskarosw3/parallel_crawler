
#include <chrono>
#include <iostream>
#include <thread>
#include <numeric>
#include <iterator>
#include <vector>
#include <cmath>
#include <curl/curl.h>
#include <climits>
#include <future>
#include <string>
#include <string_view>
#include <type_traits>
#include <fstream>

#include <regex>
#include <set>
#include <mutex>
#include <queue>
#include <condition_variable>

#include "scraper.h"
#include "queue.h"
//#include "coarse_set.h"
#include "SetList.h"

typedef std::vector<uint32_t>::const_iterator Iter;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}




void Identity(std::vector<char*> urls) {

    return;
}

void OnlyStartingWith(std::vector<std::string>* urls, std::string start) { //

    // https://en.cppreference.com/cpp/regex/regex_match
    std::string regex_start = start + "(.)*";
    std::smatch match;

    std::regex re(regex_start);
    std::vector<int> indexes_to_erase;
    for (int index = 0; index < urls->size(); index++) {
        if (!std::regex_match((*urls)[index], re )) {
            indexes_to_erase.push_back(index);
        }
    }
    for (int index = 0; index < indexes_to_erase.size(); index++) {
        urls->erase(urls->begin() + indexes_to_erase[index]- index);
    }

}

std::string FindMainURL(std::string url) {

    int start_find;
    if (url.find("http") == std::string::npos) {
        start_find = 0;
    }
    else {
        start_find = 8;
    }
    if (url.find("/", start_find) == std::string::npos) {
        return url; // we assume all are correct

    }
    else {
        return url.substr(0, url.find("/", start_find));
    }
    }


void ScraperAux(SetList& visited_sites, SafeUnboundedQueueCV<std::pair<std::string, int>>& queue , std::string core_website, bool filter_key_function) {

    //for more statistics there could be a atomic int with the current depth - not correct
    // maybe a second queue that has the same lock, but gives out the depth


    //std::cout << FindMainURL("https://www.steamculturalcapital.com/") << std::endl;
    //std::cout << FindMainURL("https://www.twitch.tv/northernlion") << std::endl;
    //currently it has errors due to atomic_int, cv mismatch, but the main idea is


    // Problem -> it's not a full BFS with the way my Queue will take care of them, and we might not get proper "shortest link", but checking if in and then check if n_stored < n_new might be costly

    while (1) {
        CURL *curl;
        std::string readBuffer;
        std::string link_match = "<a\\s+[^>]*?href=\"([^\"]*)";

        if (visited_sites.size() > 1000) {
            return;
        }


        std::pair<std::string, int> current_pair = queue.check_and_pop();

        std::string website = current_pair.first;
        int current_depth = current_pair.second;

        if (website == "") {
            return;
        }

        //visited_sites.add(website); // TODO: Check later if checking existance at this point is better

        std::set<std::string> possible_links;

        if (visited_sites.add_and_update_distance(website, current_depth, std::ref(possible_links))) {
            //if it has a lower depth, then scrape it again
            // it was in the beginning before to reduce unnessery scraping, but it might be better here with link_cache

            std::set<std::string> links = possible_links;
            std::string real_link;
            for (const auto& link : links) {
                //std::cout << link << std::endl;
                if (link.starts_with('#')) continue; //wikipedia clauses
                if (link.find("index.php") != std::string::npos) continue;  //wikipedia clauses
                if (link.starts_with('/')) {
                    real_link = core_website + link;
                }
                else{
                    real_link = link;
                }
                int tested_distance = -1;
                if (visited_sites.contains_and_distance(real_link, tested_distance)) {
                    if (tested_distance > current_depth + 1){
                        if (filter_key_function) {
                            if (real_link.starts_with(core_website)) {
                                queue.push(std::pair(real_link, current_depth +1) );
                            }
                        }
                        else {
                            queue.push(std::pair(real_link, current_depth +1) );
                        }
                    }
                    //std::cout << real_link << std::endl;
                }
                else {
                    if (filter_key_function) {
                        if (real_link.starts_with(core_website)) {
                            queue.push(std::pair(real_link, current_depth +1) );
                        }
                    }
                    else {
                        queue.push(std::pair(real_link, current_depth +1) );
                    }

                }


            }
        }
        else {





        CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
        if(result != CURLE_OK)
            return;

        curl = curl_easy_init();
        if(curl) {

            //static const char *headerfilename = "head.out";
            //FILE *headerfile;

            //headerfile = fopen(headerfilename, "wb");
            //if(!headerfile) {
            //    curl_easy_cleanup(curl);
            //    curl_global_cleanup();
            //    return -1;
            // }
            char website_char[website.length() + 1];  //required for curlopt_url
            std::copy(website.begin(), website.end(), website_char);
            website_char[website.length()] = '\0';

            curl_easy_setopt(curl, CURLOPT_URL, website_char);
            //curl_easy_setopt(curl, CURLOPT_URL, "https://pl.wikipedia.org/wiki/Braniewo");

            // For the wikipedia to work
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "User-Agent: Parallel_Crawler");
            //headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            //  If i need writing to a file: https://curl.se/libcurl/c/sepheaders.html
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            //curl_easy_setopt(curl, CURLOPT_HEADERDATA, headerfile);


            result = curl_easy_perform(curl);
            //std::cout << "readBuffer" << readBuffer << std::endl;
            if(result != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(result));
            else {

                //https://en.cppreference.com/cpp/regex/regex_search
                std::regex link_pattern(link_match);
                std::smatch match;

                std::set<std::string> links;
                std::string::const_iterator searchStart(readBuffer.cbegin());

                //std::cout << "Links:" << std::endl;
                while (std::regex_search(searchStart, readBuffer.cend(), match, link_pattern)) {
                    links.insert(match[1]);
                    searchStart = match.suffix().first;

                }



                std::string real_link;
                std::set<std::string> working_links;

                for (const auto& link : links) {
                    //std::cout << link << std::endl;
                    if (link.starts_with('#')) continue; //wikipedia clauses
                    if (link.find("index.php") != std::string::npos) continue;  //wikipedia clauses
                    if (link.starts_with('/')) {
                        real_link = core_website + link;
                    }
                    else{
                        real_link = link;
                    }
                    working_links.insert(real_link);
                    int tested_distance = -1;
                    if (visited_sites.contains_and_distance(real_link, tested_distance)) {
                        if (tested_distance > current_depth + 1){
                            if (filter_key_function) {
                                if (real_link.starts_with(core_website)) {
                                    queue.push(std::pair(real_link, current_depth +1) );
                                }
                            }
                        else {
                            queue.push(std::pair(real_link, current_depth +1) );
                        }
                        }
                        //std::cout << real_link << std::endl;
                    }
                    else {
                        if (filter_key_function) {
                            if (real_link.starts_with(core_website)) {
                                queue.push(std::pair(real_link, current_depth +1) );
                            }
                        }
                        else {
                            queue.push(std::pair(real_link, current_depth +1) );
                        }

                    }
                    visited_sites.add_links(website, working_links); // links that don't include fake links


                }


            }

            //fclose(headerfile);

            curl_easy_cleanup(curl); //TODO: Think if it might be better to setup once and just change websites

        }
        }

        //std::cout << "a" << std::endl;
    }
    return;
    // For the number of threads active at the same time it might be best to check with the
}


//-----------------------------------------------------------------------------


int Scraper(std::string website, size_t number_of_threads,  std::string file_name, bool filter_key_function) {

    // The main idea right now is to act on the website like the binary tree in the website, creating a thread for each subsequent link
    // The sub-sites would be stored in a setlist

    // it might be better to somehow get all links first



    //temporary example as a placeholder to learn how libcurl works https://curl.se/libcurl/c/simple.html



    size_t num_threads = number_of_threads;
    std::vector<std::thread> workers(number_of_threads);

    SetList visited_sites;
    std::atomic_int finished_threads = 0;
    std::condition_variable not_empty;

    // pattern for a link from https://stackoverflow.com/questions/15926142/regular-expression-for-finding-href-value-of-a-a-link

;

    SafeUnboundedQueueCV<std::pair<std::string, int>> queue;
    queue.total_threads = number_of_threads;
    queue.push({website,0});


    std::string core_website = FindMainURL(website);

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < number_of_threads; ++i) {
        workers[i] = std::thread(ScraperAux, std::ref(visited_sites), std::ref(queue),core_website, filter_key_function);

    }

    for (int i = 0; i < number_of_threads; ++i) {
        workers[i].join();
    }



    curl_global_cleanup();

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << end - start << std::endl;
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start) << std::endl;

    //https://stackoverflow.com/questions/33588266/how-to-save-txt-file-in-c
    std::ofstream outfile;
    outfile.open(file_name, std::ofstream::trunc);
    Node* curr = visited_sites.begin() -> next;

    while (curr->next != NULL) {
        outfile << curr->item << " " << curr->distance << std::endl;
        curr = curr->next;
    }


    return 0;
}