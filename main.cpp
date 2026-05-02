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

#include "queue.h"
#include "scraper.h"
#include "coarse_set.h"


int main() {

    // debug for filter functions
    // std::vector<std::string> urls;
    // urls.push_back("https://example.com");
    // urls.push_back("https://www.youtube.com");
    // urls.push_back("https://www.youtube.com//ssssss");
    // urls.push_back("aaaahttps://www.youtube.com//ssssss");
    //
    // OnlyStartingWith(&urls, "https://www.youtube.com");
    // for (auto& url : urls) {
    //     std::cout << url << std::endl;
    // }
    //
    // std::cout << urls.size() << std::endl;

    Scraper("aaa");
    return 0;
}