# parallel_crawler
Parallel Crawler made as a project for CSE305 : Concurrent and Distributed Computing at Ecole Polytechnique


For installation the folllowing packages are needed: \
curl -    \
cmake -


### Installation instruction

Instalation of the program:

git clone https://github.com/oskarosw3/parallel_crawler
mkdir build
cd build
cmake ..
make

To make the programme work on Polytechnique machines, min Cmake version was set to 3.3, but if you have up-to-date version, change the version in Cmakelist.txt to at least 3.5.

### Usage instruction

./parallel_crawler starting_website, number_of_threads, word_that_has_to_be_included, banned_word_1, banned_word_2 etc.

Example:

./parallel_crawler https://minecraft.wiki/, 20,,User,Talk,Special,?

Without the arguments it defaults to:

./parallel_crawler https://minecraft.wiki/, 10,,User,Talk,Special,?,Wikipedia,#,Forum,File,user,action,talk,section

Which allows for proper crawl of Wikipedia and Minecraft wiki without the user sites.


### Features

The program exports all the crawled websites to a txt file "scraped_websites.txt" with the name of the site, its distance from the starting website, and the parent from which this site was found. This programme guarantees that this is the lowest distance from original website, as all child-sites are rechecked if lower distance was found.

Additionally, for performance testing, for each site tested, a file is created containing the runtime for the number of threads and the number of sites.