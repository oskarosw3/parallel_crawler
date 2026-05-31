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
//#include "coarse_set.h"
#include "SetList.h"


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

    //Scraper("https://steamculturalcapital.com/", 1, "scraped_websites.txt", true);




    //   NOTES REGARDING WEBSITES:
    // grokipedia is a bad choice, as it is very poorly optimised and thus multiplies the runtime significantly
    //


    Scraper("https://minecraft.wiki/", 30, "scraped_websites.txt", true, "");
    //Scraper("https://en.wikipedia.org/wiki/Karl_Weierstrass", 20, "scraped_websites.txt", true, "");
    //Scraper("https://www.podatki.gov.pl/", 20, "scraped_websites.txt", true, "");
    return 0;
}