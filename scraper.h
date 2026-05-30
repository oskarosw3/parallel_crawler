//
// Created by oskarosw3 on 5/2/26.
//

#ifndef PARALLEL_CRAWLER_SCRAPER_H
#define PARALLEL_CRAWLER_SCRAPER_H

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

//#include "coarse_set.h"
#include "queue.h"
#include "SetList.h"

typedef std::vector<uint32_t>::const_iterator Iter;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *stream);

void Identity(std::vector<char*> urls);

void OnlyStartingWith(std::vector<std::string>* urls, std::string start);
std::string FindMainURL(std::string url);

void ScraperAux(SetList& visited_sites, SafeUnboundedQueueCV<std::string>& queue, std::string core_website, bool filter_key_function);

int Scraper(std::string website, size_t number_of_threads,  std::string file_name, bool filter_key_function);

#endif