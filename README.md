# MKBHD's Panel Gaps

This C++ program downloads images from a specified JSON API using multithreading to improve performance. It leverages the `libcurl` library for HTTP requests and `nlohmann/json` for parsing JSON data. The downloaded images are saved in a local directory, allowing you to easily access and enjoy them.

## About MKBHD's App

MKBHD's App, while designed to provide users with high-quality wallpapers, has faced criticism for its perceived value proposition. Many users feel that the app is a **rip-off**, primarily because:

- **Subscription Model**: The app often requires users to pay a subscription fee for wallpapers that are freely available elsewhere on the internet. This leads to frustration among users who believe they should not have to pay for content that can be found at no cost.

- **Limited Unique Content**: Some users argue that the app does not offer a substantial amount of unique content compared to what is available on public image hosting platforms. Many of the wallpapers are simply repackaged from existing sources.

- **User Experience**: The app's interface and user experience have also been criticized, with some users reporting that navigating through the app can be cumbersome and not intuitive.

Overall, while the app may have some redeeming qualities, many users feel that the costs associated with it do not justify the content provided.

## Features

- Fetches JSON data from a given URL.
- Extracts image URLs and downloads them concurrently using multiple threads.
- Saves images in a local "downloads" directory.
- Includes error handling and informative logging throughout the process.

## Required Libraries

To compile and run this program, you need the following libraries:

- **libcurl**: For making HTTP requests.
- **nlohmann/json**: For parsing JSON data.

### Installation of Required Libraries

1. **libcurl**:
   - On Ubuntu/Debian:
     ```bash
     sudo apt-get install libcurl4-openssl-dev
     ```
   - On MacOS:
     ```bash
     brew install curl
     ```

2. **nlohmann/json**:
   - You can install it via a package manager or directly include the header file in your project.
   - To use it via package manager, on Ubuntu:
     ```bash
     sudo apt-get install nlohmann-json3-dev
     ```

## Compilation

Use `g++` to compile the program. Make sure to link the required libraries. Here’s an example command:

```bash
g++ -std=c++17 -lcurl -pthread -o openPanelGaps secretSauce.cpp
```

Replace secretSauce.cpp with the name of your source file if it's different.

## Usage
Run the compiled program:

```bash
./openPanelGaps
```
The program will fetch JSON data from the specified URL, process it, and download images into a downloads folder created in the current directory.

## Disclaimer
This software is provided "as is", without any express or implied warranty. The author is not responsible for any damages or issues that may arise from the use of this program. Please use it responsibly and respect copyright laws regarding the images downloaded.
