#include <chrono>
#include <iostream>
#include <thread>
#include <numeric>
#include <iterator>
#include <vector>
#include <cmath>
#include <curl/curl.h>


typedef std::vector<uint32_t>::const_iterator Iter;

void ScraperAux(int site_index, int* results){

}


//-----------------------------------------------------------------------------


void Scraper(char* website) {

    // The main idea right now is to act on the website like the binary tree in the website, creating a thread for each subsequent link
    // The sub-sites would be stored in a setlist

    // it might be better to somehow get all links first and then scrape the content of the website first if it's possible

    CURL *curl = curl_easy_init();

    if(curl) {
        std::cout << "curl works" << std::endl;
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

int main() {

    Scraper("aaa");
    return 0;
}