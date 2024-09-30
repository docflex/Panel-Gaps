#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <string>
#include <filesystem>
#include <curl/curl.h>
#include "json.hpp"
#include <mutex>
#include <queue>
#include <condition_variable>

using json = nlohmann::json;
namespace fs = std::filesystem;

// Mutex and condition variables for thread synchronization
std::mutex queueMutex;
std::condition_variable condition;
bool finished = false;
std::queue<std::pair<std::string, std::string>> taskQueue; // Holds tasks (image URL and filename)

// Callback function for libcurl to write received data to a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to download an image and save it to a file
void downloadImage(const std::string& url, const std::string& filePath) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init(); // Initialize curl session
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // Set URL
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); // Set callback to capture data
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); // Data to write
        res = curl_easy_perform(curl); // Perform the request

        if (res != CURLE_OK) {
            std::cerr << "💥 Uh-oh! Couldn't download image: " << curl_easy_strerror(res) << std::endl;
        } else {
            // Write the downloaded image to the file system
            std::ofstream file(filePath, std::ios::binary);
            if (file) {
                file.write(readBuffer.c_str(), readBuffer.size());
                std::cout << "🎉 Image successfully saved at: " << filePath << std::endl;
            } else {
                std::cerr << "❌ Error: Could not write to file: " << filePath << std::endl;
            }
        }
        curl_easy_cleanup(curl); // Clean up the curl session
    }
}

// Function to fetch JSON from the given URL
std::string fetchJson(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init(); // Initialize curl session
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // Set URL for JSON
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); // Set callback to write JSON data
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); // Write received data into the buffer
        res = curl_easy_perform(curl); // Perform the request

        if (res != CURLE_OK) {
            std::cerr << "🤔 Something went wrong while fetching JSON: " << curl_easy_strerror(res) << std::endl;
            return "";
        }
        curl_easy_cleanup(curl); // Clean up curl session
    }
    return readBuffer; // Return the received JSON data as a string
}

// Worker function for threads in the thread pool
void worker() {
    while (true) {
        std::pair<std::string, std::string> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [] { return !taskQueue.empty() || finished; });

            if (finished && taskQueue.empty()) break; // Exit condition for threads

            task = taskQueue.front(); // Get the next task
            taskQueue.pop();
        }

        // Process the task (download the image)
        downloadImage(task.first, task.second);
    }
}

// Function to download images from the JSON data using a thread pool
void downloadImagesFromJson(const json& data, const std::string& downloadDir, int threadCount = 4) {
    int fileIndex = 1;

    // Spawn a thread pool
    std::vector<std::thread> threadPool;
    for (int i = 0; i < threadCount; ++i) {
        threadPool.emplace_back(worker); // Start worker threads
    }

    // Prepare the tasks (URL and file paths)
    for (auto& [key, subproperty] : data.items()) {
        if (subproperty.contains("dhd")) {
            std::string imageUrl = subproperty["dhd"];
            std::cout << "🔎 Found image URL: " << imageUrl << std::endl;

            // Extract the file extension and remove query parameters
            std::string ext;
            size_t dotPos = imageUrl.find_last_of('.');
            size_t queryPos = imageUrl.find('?');

            if (dotPos != std::string::npos) {
                if (queryPos != std::string::npos) {
                    ext = imageUrl.substr(dotPos, queryPos - dotPos);  // Extract only the extension without query parameters
                } else {
                    ext = imageUrl.substr(dotPos);  // No query parameters, just extract the extension
                }
            }

            // If extension isn't valid, default to .jpg
            if (ext != ".jpg" && ext != ".png") {
                ext = ".jpg";
            }

            // Create the filename and path
            std::string filename = std::to_string(fileIndex++) + ext;
            std::string filePath = fs::path(downloadDir) / filename;

            // Add task to queue
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                taskQueue.emplace(imageUrl, filePath); // Add the task (URL, file path) to the queue
            }
            condition.notify_one(); // Notify a thread to start processing
        }
    }

    // Indicate that we are done adding tasks
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        finished = true;
    }
    condition.notify_all(); // Wake up all threads to finish their tasks

    // Join all threads
    for (auto& t : threadPool) {
        if (t.joinable()) {
            t.join(); // Wait for the threads to finish
        }
    }
}

int main() {
    // Fun Ascii art banner
    std::cout << R"(
  ________  ________  ________   _______   ___               ________  ________  ________  ________      
|\   __  \|\   __  \|\   ___  \|\  ___ \ |\  \             |\   ____\|\   __  \|\   __  \|\   ____\     
\ \  \|\  \ \  \|\  \ \  \\ \  \ \   __/|\ \  \            \ \  \___|\ \  \|\  \ \  \|\  \ \  \___|_    
 \ \   ____\ \   __  \ \  \\ \  \ \  \_|/_\ \  \            \ \  \  __\ \   __  \ \   ____\ \_____  \   
  \ \  \___|\ \  \ \  \ \  \\ \  \ \  \_|\ \ \  \____        \ \  \|\  \ \  \ \  \ \  \___|\|____|\  \  
   \ \__\    \ \__\ \__\ \__\\ \__\ \_______\ \_______\       \ \_______\ \__\ \__\ \__\     ____\_\  \ 
    \|__|     \|__|\|__|\|__| \|__|\|_______|\|_______|        \|_______|\|__|\|__|\|__|    |\_________\
                                                                                            \|_________|
                                                                                                        
                                                                                                        )";

    std::cout << "\n🤑 Time to grab some epic wallpapers from your favorite cash-grabbing app!\n";

    // URL to fetch JSON from
    std::string url = "https://storage.googleapis.com/panels-api/data/20240916/media-1a-i-p~s";
    
    // Fetch the JSON
    std::string jsonData = fetchJson(url);
    if (jsonData.empty()) {
        std::cerr << "💔 Oh no! We couldn't fetch any data. Try again later!" << std::endl;
        return 1;
    }

    // Parse the JSON
    json parsedData = json::parse(jsonData);
    if (!parsedData.contains("data")) {
        std::cerr << "💢 The JSON file seems to be missing a 'data' section. Are we being trolled?" << std::endl;
        return 1;
    }

    // Create the 'downloads' directory if it doesn't exist
    std::string downloadDir = "downloads";
    if (!fs::exists(downloadDir)) {
        fs::create_directory(downloadDir);
        std::cout << "📁 No folder? No problem! Created 'downloads' directory for you!" << std::endl;
    }

    // Start downloading images using the thread pool
    std::cout << "🚀 Firing up the download engine with multithreaded awesomeness! Hold tight...\n";
    downloadImagesFromJson(parsedData["data"], downloadDir, 8);  // Pass the desired number of threads (e.g., 8)

    std::cout << "🎉 Done! All images have been downloaded to the 'downloads' folder. Enjoy your new wallpapers!\n";

    return 0;
}

