#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define MAX_WORD_LENGTH 100
#define MAX_LINE_LENGTH 1000
#define MAX_LINES 1000

// Metin dosyasını okuyarak her satırda bir kelime olduğunu kontrol eden fonksiyon
int readTextFile(const char *filename, char lines[MAX_LINES][MAX_LINE_LENGTH], int *numLines) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Dosya açılamadı: %s\n", filename);
        return -1; // Hata kodu
    }

    int lineCount = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strlen(line) > 0 && line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0'; // Yeni satır karakterini kaldır
        }

        // Satır uzunluğunu kontrol et
        if (strlen(line) >= MAX_WORD_LENGTH) {
            fprintf(stderr, "Hata: Satır %d çok uzun.\n", lineCount + 1);
            continue; // Bu satırı atla ve sonraki satıra geç
        }

        strcpy(lines[lineCount], line);
        lineCount++;

        // Max satır sayısını kontrol et
        if (lineCount >= MAX_LINES) {
            fprintf(stderr, "Hata: Çok fazla satır.\n");
            break; // Max satır sayısına ulaşıldı, döngüyü sonlandır
        }
    }

    *numLines = lineCount;
    fclose(file);
    return 0; // Başarı kodu
}

// Merge Sort
void merge(char arr[][MAX_LINE_LENGTH], int left, int middle, int right) {
    int i, j, k;
    int n1 = middle - left + 1;
    int n2 = right - middle;

    char L[n1][MAX_LINE_LENGTH], R[n2][MAX_LINE_LENGTH];

    for (i = 0; i < n1; i++)
        strcpy(L[i], arr[left + i]);
    for (j = 0; j < n2; j++)
        strcpy(R[j], arr[middle + 1 + j]);

    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2) {
        if (strcmp(L[i], R[j]) <= 0) {
            strcpy(arr[k], L[i]);
            i++;
        } else {
            strcpy(arr[k], R[j]);
            j++;
        }
        k++;
    }

    while (i < n1) {
        strcpy(arr[k], L[i]);
        i++;
        k++;
    }

    while (j < n2) {
        strcpy(arr[k], R[j]);
        j++;
        k++;
    }
}

void mergeSort(char arr[][MAX_LINE_LENGTH], int left, int right, int numThreads) {
    if (left < right) {
        int middle = left + (right - left) / 2;

        #pragma omp parallel sections num_threads(numThreads)
        {
            #pragma omp section
            {
                mergeSort(arr, left, middle, numThreads);
            }
            #pragma omp section
            {
                mergeSort(arr, middle + 1, right, numThreads);
            }
        }

        merge(arr, left, middle, right);
    }
}


// Quick Sort
int partition(char arr[][MAX_LINE_LENGTH], int low, int high) {
    char pivot[MAX_LINE_LENGTH];
    strcpy(pivot, arr[high]);
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        if (strcmp(arr[j], pivot) < 0) {
            i++;
            char temp[MAX_LINE_LENGTH];
            strcpy(temp, arr[i]);
            strcpy(arr[i], arr[j]);
            strcpy(arr[j], temp);
        }
    }
    char temp[MAX_LINE_LENGTH];
    strcpy(temp, arr[i + 1]);
    strcpy(arr[i + 1], arr[high]);
    strcpy(arr[high], temp);
    return (i + 1);
}

void quickSort(char arr[][MAX_LINE_LENGTH], int low, int high, int numThreads) {
    if (low < high) {
        int pi = partition(arr, low, high);

        #pragma omp parallel sections num_threads(numThreads)
        {
            #pragma omp section
            {
                quickSort(arr, low, pi - 1, numThreads);
            }
            #pragma omp section
            {
                quickSort(arr, pi + 1, high, numThreads);
            }
        }
    }
}


int main() {
    char inputFile[MAX_WORD_LENGTH];
    char outputFile[MAX_WORD_LENGTH];
    int numThreads;
    char algorithm[MAX_WORD_LENGTH];

    // Kullanıcıdan girdileri al
    printf("Input file: ");
    scanf("%s", inputFile);
    printf("Output file: ");
    scanf("%s", outputFile);
    printf("# of threads: ");
    scanf("%d", &numThreads);
    printf("Algorithm (merge/quick): ");
    scanf("%s", algorithm);

    char lines[MAX_LINES][MAX_LINE_LENGTH];
    int numLines = 0;

    // Metin dosyasını oku
    if (readTextFile(inputFile, lines, &numLines) != 0) {
        return 1; // Hata durumunda çıkış yap
    }

    // Sıralama algoritmasını belirle ve uygula
    if (strcmp(algorithm, "merge") == 0) {
        mergeSort(lines, 0, numLines - 1, numThreads);
    } else if (strcmp(algorithm, "quick") == 0) {
        quickSort(lines, 0, numLines - 1, numThreads);
    } else {
        printf("Hatalı sıralama algoritması!\n");
        return 1;
    }

    // Sıralanmış satırları çıktı dosyasına yaz
    FILE *outputFilePtr = fopen(outputFile, "w");
    if (outputFilePtr == NULL) {
        fprintf(stderr, "Dosya açılamadı: %s\n", outputFile);
        return 1;
    }

    for (int i = 0; i < numLines; i++) {
        fprintf(outputFilePtr, "%s\n", lines[i]);
    }

    fclose(outputFilePtr);
    printf("Sıralama tamamlandı. Çıktı dosyası: %s\n", outputFile);

    return 0;
}
