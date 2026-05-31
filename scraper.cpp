
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
#include <re2/re2.h>
#include <set>
#include <mutex>
#include <queue>
#include <condition_variable>

#include "scraper.h"
#include "queue.h"
//#include "coarse_set.h"
#include <strings.h>

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

void OnlyStartingWith(std::set<std::string>* urls, std::string start) { //

    std::erase_if(*urls, [&start](const std::string& url) {
        return !(url.starts_with(start) || url.starts_with('/') );
    });
}



std::string FindMainURL(const std::string& url) {

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


void ScraperAux(SetList& visited_sites, SafeUnboundedQueueCV<std::pair<std::pair<std::string, int>, std::string>>& queue ,
    std::string core_website, bool filter_key_function, std::string filter_word, int nb_of_sites) {

    //for more statistics there could be a atomic int with the current depth - not correct
    // maybe a second queue that has the same lock, but gives out the depth


    //std::cout << FindMainURL("https://www.steamculturalcapital.com/") << std::endl;
    //std::cout << FindMainURL("https://www.twitch.tv/northernlion") << std::endl;
    //currently it has errors due to atomic_int, cv mismatch, but the main idea is


    // Problem -> it's not a full BFS with the way my Queue will take care of them, and we might not get proper "shortest link", but checking if in and then check if n_stored < n_new might be costly
    CURL *curl;
    std::string readBuffer;
    re2::RE2 link_pattern("<a\\s+[^>]*?href=\"([^\"]*)\"");



    CURLcode result = curl_global_init(CURL_GLOBAL_ALL); //for some reason this being here and not outside speeds up by x2
    if(result != CURLE_OK) {
        //std::cout << "error for website " << core_website << std::endl;
        return;
    }



    while (1) {

        readBuffer.clear();



        //if (visited_sites.nb_nodes % 1000 == 0) {
        //    std::printf("%d",visited_sites.nb_nodes.fetch_add(0));
        //}



        if (visited_sites.nb_nodes > nb_of_sites) {
            curl_easy_cleanup(curl);
            return;
        }


        std::pair<std::pair<std::string, int>, std::string> current_pair = queue.check_and_pop();

        std::pair<std::string, int> new_pair = current_pair.first;
        std::string parent = current_pair.second;

        std::string website = new_pair.first;
        int current_depth = new_pair.second;

        if (website == "")  {
            curl_easy_cleanup(curl);
            return;
        }

        //visited_sites.add(website); // TODO: Check later if checking existance at this point is better

        std::set<std::string> possible_links;

        if (visited_sites.add_and_update_distance(website, current_depth, std::ref(possible_links), parent)) {
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
                                queue.push(std::pair(std::pair(real_link, current_depth +1), website)  );
                            }
                        }
                        else {
                            queue.push(std::pair(std::pair(real_link, current_depth +1), website) );
                        }
                    }
                    //std::cout << real_link << std::endl;
                }
                else {
                    if (filter_key_function) {
                        if (real_link.starts_with(core_website)) {
                            queue.push(std::pair(std::pair(real_link, current_depth +1), website)  );
                        }
                    }
                    else {
                        queue.push(std::pair(std::pair(real_link, current_depth +1), website)  );
                    }

                }


            }
        }
        else {


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

            curl_easy_setopt(curl, CURLOPT_URL, website.c_str());
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

            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L); //for multithreading

            result = curl_easy_perform(curl);
            //std::cout << "readBuffer" << readBuffer << std::endl;
            if(result != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(result));
            else {

                //https://en.cppreference.com/cpp/regex/regex_search

                re2::StringPiece input(readBuffer); //change from regex to re2 (faster)
                std::string extracted_link;

                std::set<std::string> links;
                std::string::const_iterator searchStart(readBuffer.cbegin());

                //std::cout << "Links:" << std::endl;
                while (re2::RE2::FindAndConsume(&input, link_pattern, &extracted_link)) {
                    links.insert(extracted_link);
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

                }

                OnlyStartingWith(&working_links, core_website);

                for (const auto& real_link : working_links) {
                    //std::cout << real_link << std::endl;
                    int tested_distance = -1;
                    if (visited_sites.contains_and_distance(real_link, tested_distance)) {
                        if (tested_distance > current_depth + 1){
                            if (filter_key_function) {
                                if (real_link.starts_with(core_website)) {
                                    if (real_link.find(filter_word) != std::string::npos || filter_word == "") {
                                        queue.push(std::pair(std::pair(real_link, current_depth +1), website) );
                                    }

                                }
                            }
                        else {
                            if (real_link.find(filter_word) != std::string::npos || filter_word == "") {
                                queue.push(std::pair(std::pair(real_link, current_depth +1), website) );
                            }
                        }
                        }
                        //std::cout << real_link << std::endl;
                    }
                    else {
                        if (filter_key_function) {
                            if (real_link.starts_with(core_website)) {
                                if (real_link.find(filter_word) != std::string::npos || filter_word == "") {
                                    queue.push(std::pair(std::pair(real_link, current_depth +1), website) );
                                }
                            }
                        }
                        else {
                            if (real_link.find(filter_word) != std::string::npos || filter_word == "") {
                                queue.push(std::pair(std::pair(real_link, current_depth +1), website) );
                            }
                        }

                    }
                    visited_sites.add_links(website, working_links); // links that don't include fake links


                }


            }

            //fclose(headerfile);

            //curl_easy_cleanup(curl); //DONE: Think if it might be better to setup once and just change websites

        }
        }

        //std::cout << "a" << std::endl;
    }
    return;
    // For the number of threads active at the same time it might be best to check with the
}


//-----------------------------------------------------------------------------


int Scraper(std::string website, size_t number_of_threads,  std::string file_name, bool filter_key_function, std::string filter_word) {

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

    SafeUnboundedQueueCV<std::pair<std::pair<std::string, int>, std::string>> queue;
    queue.total_threads = number_of_threads;
    queue.push(std::pair(std::pair(website, 0), "root"));


    std::string core_website = FindMainURL(website);

    int nb_of_sites = 40000;
    auto start = std::chrono::high_resolution_clock::now();


    for (size_t i = 0; i < number_of_threads; ++i) {
        workers[i] = std::thread(ScraperAux, std::ref(visited_sites), std::ref(queue),core_website, filter_key_function,
            filter_word, nb_of_sites);

    }

    for (int i = 0; i < number_of_threads; ++i) {
        workers[i].join();
    }



    curl_global_cleanup();

    auto end = std::chrono::high_resolution_clock::now();

    //std::cout << end - start << std::endl;
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start)/1000.0 << std::endl;

    //https://stackoverflow.com/questions/33588266/how-to-save-txt-file-in-c
    std::ofstream outfile;
    outfile.open(file_name, std::ofstream::trunc);
    Node* curr = visited_sites.begin() -> next;
    outfile << "site,distance,parent" << std::endl;
    while (curr->next != NULL) {
        outfile << curr->item << "," << curr->distance << "," << curr->parent  << std::endl;
        curr = curr->next;
    }

    std::string website_file = core_website;

    for (size_t i = 0; i < core_website.size(); ++i) {
        if (core_website[i] == '.' || core_website[i] == '/' || core_website[i] == ':') {
            website_file[i] = '-';
        }
        else {
            website_file[i] = core_website[i];
        }
    }
    website_file += ".txt";

    std::ofstream website_outfile;
    website_outfile.open(website_file, std::ofstream::app );

    if (website_outfile.tellp() == 0) {
        website_outfile << "sites_scraped,number_of_threads,time" << std::endl;
    }
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    website_outfile <<  nb_of_sites << ',' << num_threads<< ',' << (microseconds)/1000000.0  << std::endl;


    return 0;
}