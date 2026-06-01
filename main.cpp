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


int main(int argc, char* argv[]) {

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


    std::vector <std::string> bad_words;

    // for minecraft wiki
    bad_words.push_back("section");
    bad_words.push_back("talk");
    bad_words.push_back("action");
    bad_words.push_back("user");
    bad_words.push_back("File");
    bad_words.push_back("Forum");
    bad_words.push_back("?");
    bad_words.push_back("#");
    bad_words.push_back("User");
    bad_words.push_back("Talk");
    bad_words.push_back("Special");
    bad_words.push_back("Wikipedia");


    // for wikipedia
    //bad_words.push_back("index.php");

    std::string website, filter_word;
    int number_of_threads;
    std::vector<std::string> bad_words_new;
    if (argc > 1) {
        website = argv[1];
    }
    else {
        website = "https://minecraft.wiki/";
    }

    if (argc > 2) {
        number_of_threads = std::stoi(argv[2]);
    }
    else {
        number_of_threads = 10;
    }

    if (argc > 3) {
        filter_word = argv[3];
    }
    else {
        filter_word = "";
    }

    if (argc > 4) {
        for (int i = 4; i < argc; i++) {
            bad_words_new.push_back(argv[i]);
        }
    }
    else {
        for (int i = 0; i < bad_words.size(); i++) {
            bad_words_new.push_back(bad_words[i]);
        }
    }

    Scraper(website, number_of_threads, filter_word, bad_words_new);
    //Scraper("https://en.wikipedia.org/wiki/Karl_Weierstrass", 20, "scraped_websites.txt", true, "");
    //Scraper("https://www.podatki.gov.pl/", 20, "scraped_websites.txt", true, "");
    return 0;
}