#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <regex.h>

struct WebBuffer {
    char *data;
    size_t size;
};

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

void run_search(const char *target_url) {
    CURL *curl;
    CURLcode res;
    struct WebBuffer chunk = { malloc(1), 0 };
    const char *pattern = "<h4[^>]*>([^<]*)</h4>[^<]*<p[^>]*>([^<]*)</p>";
    regex_t regex;
    regmatch_t matches[3];

    curl = curl_easy_init();
    if(!curl) return;

    regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE);

    curl_easy_setopt(curl, CURLOPT_URL, target_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

    res = curl_easy_perform(curl);

    if(res == CURLE_OK) {
        printf("\033[H\033[2J"); // Clear Screen
        char *search_ptr = chunk.data;
        
        // Skip header lines for accessibility
        for (int i = 0; i < 77 && search_ptr != NULL; i++) {
            search_ptr = strchr(search_ptr, '\n');
            if (search_ptr) search_ptr++;
        }

        if (search_ptr) {
            int found = 0;
            while (regexec(&regex, search_ptr, 3, matches, 0) == 0) {
                found = 1;
                printf("\n[TITLE]: %.*s\n", (int)(matches[1].rm_eo - matches[1].rm_so), search_ptr + matches[1].rm_so);
                printf("[INFO] : %.*s\n", (int)(matches[2].rm_eo - matches[2].rm_so), search_ptr + matches[2].rm_so);
                printf("--------------------------------------------------\n");
                search_ptr += matches[0].rm_eo;
            }
            if(!found) printf("\nNo movies found here.\n");
        }
    }
    
    curl_easy_cleanup(curl);
    regfree(&regex);
    free(chunk.data);
    printf("\nPress Enter to return to menu...");
    getchar(); getchar(); // Pause so user can read
}

int main(void) {
    int choice;
    char movie_name[100];
    char search_url[256];

    while(1) {
        printf("\033[H\033[2J"); // Clear Menu Screen
        printf("=== ACCESSIBLE MOVIE SEARCH ===\n");
        printf("1. View Main Page (Latest)\n");
        printf("2. Search by Movie Title\n");
        printf("0. Exit\n");
        printf("Choice: ");
        if(scanf("%d", &choice) != 1) break;

        switch(choice) {
            case 1:
                run_search("https://yts-official.org");
                break;
            case 2:
                printf("Enter movie name: ");
                scanf("%s", movie_name); 
                // Build the search URL
                snprintf(search_url, sizeof(search_url), "https://yts-official.orgbrowse-movies/%s/all/all/0/latest/0/all", movie_name);
                run_search(search_url);
                break;
            case 0:
                return 0;
            default:
                printf("Invalid choice.\n");
        }
    }
    return 0;
}

