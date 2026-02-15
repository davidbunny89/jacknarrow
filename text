#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <regex.h>

// 1. Structure to hold the website data in RAM
struct WebBuffer {
    char *data;
    size_t size;
};

// 2. Callback function: Tells Curl to save data to our buffer instead of printing it
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t realsize = size * nmemb;
    struct WebBuffer *mem = (struct WebBuffer *)userdata;

    char *ptr_new = realloc(mem->data, mem->size + realsize + 1);
    if(ptr_new == NULL) return 0; 

    mem->data = ptr_new;
    memcpy(&(mem->data[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0; 

    return realsize;
}

int main(void) {
    CURL *curl;
    CURLcode res;
    struct WebBuffer chunk = { malloc(1), 0 };

    // 3. Regex Pattern: Refined to capture text inside tags safely
    // Group 1: Title, Group 2: Description
    const char *pattern = "<h4[^>]*>([^<]*)</h4>[^<]*<p[^>]*>([^<]*)</p>";
    regex_t regex;
    regmatch_t matches[3]; 

    // Initialize Curl
    curl = curl_easy_init();
    if(!curl) return 1;

    // Compile Regex (Extended and Case-Insensitive)
    // Removed REG_DOTALL to ensure compatibility with standard POSIX regex.h
    if (regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE)) {
        fprintf(stderr, "Regex failed to compile\n");
        return 1;
    }

    // Set Curl Options
    curl_easy_setopt(curl, CURLOPT_URL, "https://yts-official.org");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");

    // Execute the fetch
    res = curl_easy_perform(curl);

    if(res == CURLE_OK) {
        // 4. PRE-PROCESSING FOR ACCESSIBILITY
        char *search_ptr = chunk.data;
        int lines_to_skip = 77;

        // Skip the first 77 lines of header/metadata junk
        for (int i = 0; i < lines_to_skip && search_ptr != NULL; i++) {
            search_ptr = strchr(search_ptr, '\n');
            if (search_ptr) {
                search_ptr++; 
            }
        }

        // Only start searching if we didn't run out of file
        if (search_ptr) {
            printf("\033[H\033[2J"); // Clear screen for clean output
            
            // 5. Automated Extraction Loop
            while (regexec(&regex, search_ptr, 3, matches, 0) == 0) {
                // Get offsets for Title
                int t_start = matches[1].rm_so;
                int t_end = matches[1].rm_eo;

                // Get offsets for Paragraph
                int p_start = matches[2].rm_so;
                int p_end = matches[2].rm_eo;

                // Print clean, accessible output
                printf("\n[TITLE]: %.*s\n", t_end - t_start, search_ptr + t_start);
                printf("[INFO] : %.*s\n", p_end - p_start, search_ptr + p_start);
                printf("--------------------------------------------------\n");

                // Advance pointer past the WHOLE match
                search_ptr += matches[0].rm_eo;
            }
        }
    } else {
        fprintf(stderr, "Curl failed: %s\n", curl_easy_strerror(res));
    }

    // 6. Cleanup (Outside all loops)
    curl_easy_cleanup(curl);
    regfree(&regex);
    free(chunk.data);

    return 0;
}

