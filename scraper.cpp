
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


#include <regex>
#include <set>
#include <mutex>
#include <queue>
#include <condition_variable>

#include "scraper.h"
#include "queue.h"
#include "coarse_set.h"


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


void ScraperAux(CoarseSetList& visited_sites, SafeUnboundedQueueCV<std::string>& queue, std::atomic_int finished_threads,
    std::condition_variable& not_empty, size_t total_threads ) {

    //for more statistics there could be a atomic int with the current depth - not correct
    // maybe a second queue that has the same lock, but gives out the depth




    //currently it has errors due to atomic_int, cv mismatch, but the main idea is

    // TODO: while (atomic_int finished_threads < total threads) - so the threads might start again even after the queue was once empty

    // TODO: then at the end we have cv that wakes up threads after the queue is no longer empty, and also after a thread finishes so all threads might finish

    CURL *curl;
    std::string readBuffer;
    std::string link_match = "<a\\s+[^>]*?href=\"([^\"]*)";

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

        curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");
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
        std::cout << readBuffer << std::endl;
        if(result != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result));
        else {

            //https://en.cppreference.com/cpp/regex/regex_search
            std::regex link_pattern(link_match);
            std::smatch match;

            std::set<std::string> links;
            std::string::const_iterator searchStart(readBuffer.cbegin());

            std::cout << "Links:" << std::endl;
            while (std::regex_search(searchStart, readBuffer.cend(), match, link_pattern)) {
                links.insert(match[1]);
                searchStart = match.suffix().first;
            }

            for (const auto& link : links) {
                std::cout << link << std::endl;
            }


        }

        //fclose(headerfile);

        curl_easy_cleanup(curl);

    }

    std::cout << std::endl;
    return;
    // For the number of threads active at the same time it might be best to check with the
}


//-----------------------------------------------------------------------------


int Scraper(std::string website) {

    // The main idea right now is to act on the website like the binary tree in the website, creating a thread for each subsequent link
    // The sub-sites would be stored in a setlist

    // it might be better to somehow get all links first



    //temporary example as a placeholder to learn how libcurl works https://curl.se/libcurl/c/simple.html



    size_t num_threads =  3;
    std::vector<std::thread> workers(num_threads);

    CoarseSetList visited_sites;
    std::atomic_int finished_threads = 0;
    std::condition_variable not_empty;

    // pattern for a link from https://stackoverflow.com/questions/15926142/regular-expression-for-finding-href-value-of-a-a-link

;

    SafeUnboundedQueueCV<std::string> queue;
    queue.push(website);

    for (size_t i = 0; i < num_threads; ++i) {
        workers[i] = std::thread(ScraperAux, std::ref(visited_sites), std::ref(queue), finished_threads,
            &not_empty, num_threads);

    }



    curl_global_cleanup();
    return 0;
}