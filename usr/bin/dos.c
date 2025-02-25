
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <curl/curl.h>
#include <signal.h>
#include <time.h>

#define MAX_THREADS 10
#define REQUESTS_PER_THREAD 100

pthread_mutex_t lock;
int request_counter = 0;
volatile int running = 1; // Variable para controlar la ejecución del hilo de monitoreo

typedef struct {
    char url[256];
    int thread_id;
    int requests_per_thread;
} ThreadData;

// Lista de User-Agents
const char *user_agents[] = {
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.1.1 Safari/605.1.15",
    "Mozilla/5.0 (Linux; Android 10; Pixel 3 XL) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.114 Mobile Safari/537.36",
    "Mozilla/5.0 (iPhone; CPU iPhone OS 14_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.0 Mobile/15E148 Safari/604.1",
    "Mozilla/5.0 (Linux; Android 11; SM-G991B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.114 Mobile Safari/537.36"
};

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    return size * nmemb; // Ignorar la respuesta
}

void *http_request(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error initializing CURL in thread %d\n", data->thread_id);
        pthread_exit(NULL);
    }

    curl_easy_setopt(curl, CURLOPT_URL, data->url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // Establecer un tiempo de espera de 10 segundos

    // Seleccionar un User-Agent aleatorio
    int user_agent_index = rand() % (sizeof(user_agents) / sizeof(user_agents[0]));
    curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agents[user_agent_index]);

    for (int i = 0; i < data->requests_per_thread; i++) {
        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            pthread_mutex_lock(&lock);
            request_counter++;
            pthread_mutex_unlock(&lock);
        } else {
            fprintf(stderr, "CURL error in thread %d: %s\n", data->thread_id, curl_easy_strerror(res));
        }
        usleep(100000); // Evitar sobrecarga
    }
    curl_easy_cleanup(curl);
    pthread_exit(NULL);
}

void *monitor(void *arg) {
    int prev_count = 0;
    while (running) {
        sleep(1);
        pthread_mutex_lock(&lock);
        printf("Requests Sent: %d (+%d/s)\n", request_counter, request_counter - prev_count);
        prev_count = request_counter;
        pthread_mutex_unlock(&lock);
    }
    pthread_exit(NULL);
}

void handle_signal(int signal) {
    if (signal == SIGINT) {
        running = 0; // Detener el monitoreo
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <URL> <num_threads> <requests_per_thread>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[2]);
    int requests_per_thread = atoi(argv[3]);

    if (num_threads > MAX_THREADS) {
        fprintf(stderr, "El número máximo de hilos es %d\n", MAX_THREADS);
        return 1;
    }

    pthread_t threads[MAX_THREADS], monitor_thread;
    ThreadData thread_data[MAX_THREADS];
    pthread_mutex_init(&lock, NULL);

    // Manejo de señales
    signal(SIGINT, handle_signal);

    // Inicializar la semilla para la generación de números aleatorios
    srand(time(NULL));

    // Conectar a la VPN (ejemplo, descomentar y ajustar según tu configuración)
    // system("openvpn --config /path/to/your/config.ovpn &");

    // Crear hilos para enviar solicitudes HTTP
    for (int i = 0; i < num_threads; i++) {
        snprintf(thread_data[i].url, sizeof(thread_data[i].url), "%s", argv[1]);
        thread_data[i].thread_id = i;
        thread_data[i].requests_per_thread = requests_per_thread;
        if (pthread_create(&threads[i], NULL, http_request, &thread_data[i]) != 0) {
            fprintf(stderr, "Error creando hilo %d\n", i);
            return 1;
        }
    }

    // Crear hilo de monitoreo
    if (pthread_create(&monitor_thread, NULL, monitor, NULL) != 0) {
        fprintf(stderr, "Error creando hilo de monitoreo\n");
        return 1;
    }

    // Esperar a que todos los hilos de solicitudes terminen
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Finalizar el hilo de monitoreo
    running = 0; // Señalar al hilo de monitoreo que debe detenerse
    pthread_join(monitor_thread, NULL); // Esperar a que el hilo de monitoreo termine

    pthread_mutex_destroy(&lock);
    printf("Total Requests Sent: %d\n", request_counter);
    return 0;
}
