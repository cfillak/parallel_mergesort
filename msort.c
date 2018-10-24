/** @file msort.c */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define CHAR_MEM 512	// nu

typedef struct _seg_t {

	int *start;
	int size;

} seg_t;

typedef struct _merge_seg_t {

	int *seg1;
	int *seg2;
	int size1;
	int size2;

} merge_seg_t;

int compare(const void *a, const void *b) {
	return ( *(int*)a - *(int*)b );
}

void * merge(void * input) {

	merge_seg_t *m_seg = (merge_seg_t *)input;

	int *left = m_seg->seg1;
	int *right = m_seg->seg2;
	int *begin = left;

	int L_size = m_seg->size1;
	int R_size = m_seg->size2;
	int L_count = 0;
	int R_count = 0;
	int * temp = malloc(sizeof(int) *(L_size + R_size));

	int dupes = 0;
	int j = 0;

	while (L_count < L_size && R_count < R_size) {

		if ( *left < *right ) {

			temp[j] = *left;
			L_count++;
			left++;

		} else {

			if (*left == *right) dupes++;
			temp[j] = *right;
			R_count++;
			right++;

		}

		j++;

	}


	while (L_count < L_size)  {

		temp[j] = *left;
		L_count++;
		left++;
		j++;
	}

	while (R_count < R_size) {

		temp[j] = *right;
		R_count++;
		right++;
		j++;

	}

	memcpy(begin, temp, (sizeof(int)* (L_size + R_size)));
	free(temp);

	fprintf(stderr, "Merged %d and %d elements with %d duplicates\n", L_size, R_size, dupes);
	return NULL;

}

void * sort(void * p) {

	qsort( ((seg_t *)p)->start, ((seg_t *)p)->size, sizeof(int), compare);
	fprintf(stderr, "Sorted %d elements. \n", ((seg_t *)p)->size);
	return NULL;

}

int main(int argc, char **argv) {
	if (argc < 2) {
		return 0;
	}

	int segment_count = atoi(argv[1]);

	char *line_ptr;
	size_t size = CHAR_MEM;

	char *line = malloc(CHAR_MEM);

	int input_count = 0;
	int input_max = 1024;
	int *list = malloc(sizeof(int) * input_max);

	while (1) {
		line_ptr = fgets(line, size, stdin);

		if (line_ptr == NULL) {
			break;
		}

		list[input_count] = atoi(line_ptr);
		input_count++;

		if (input_count == input_max-1) {
			input_max *= 2;
			list = realloc(list, sizeof(int)*input_max);
		}
	}

	int values_per_segment;

	if (input_count%segment_count == 0) {
		values_per_segment = input_count/segment_count;
	} else {
		values_per_segment = (input_count/segment_count) + 1;
	}


	pthread_t *threads = malloc(sizeof(pthread_t) * segment_count);
	seg_t *sort_segments = malloc(sizeof(seg_t) * segment_count);
	merge_seg_t *merge_segments = malloc(sizeof(merge_seg_t) * segment_count);


	int i;

	for (i = 0; i < segment_count; i++) {

		sort_segments[i].start = &list[0 + i*values_per_segment];

		if (i == (segment_count-1)) {
			sort_segments[i].size = input_count - (values_per_segment*i);
		} else {
			sort_segments[i].size = values_per_segment;
		}

		pthread_create(&threads[i], NULL, sort, (void *)&sort_segments[i]);
	}

	for (i = 0; i < segment_count; i++) {
		pthread_join(threads[i], NULL);
	}


	int merge_segment_count = segment_count;


	while (merge_segment_count > 1) {
		int merges_per_round = merge_segment_count/2;

		for (i = 0; i < merges_per_round; i++) {
			merge_segments[i].seg1 = list + (i*values_per_segment*2);
			merge_segments[i].size1 = values_per_segment;
			merge_segments[i].seg2 = list + (i*values_per_segment*2) + values_per_segment;

			if ( 2*values_per_segment*(i+1) > input_count) {
				merge_segments[i].size2 = input_count - (values_per_segment*(2*i + 1));
			} else {
				merge_segments[i].size2 = values_per_segment;
			}

			pthread_create(&threads[i], NULL, merge, (void *)&merge_segments[i]);

		}

		for (i = 0; i < merges_per_round; i++) {
			pthread_join(threads[i], NULL);
			merge_segment_count--;
		}

		values_per_segment *= 2;
	}


	for (i = 0; i < input_count; i++) {
		printf("%i\n", list[i]);
	}

	free(list);
	free(threads);
	free(sort_segments);
	free(line);
	free(merge_segments);

	return 0;

}
