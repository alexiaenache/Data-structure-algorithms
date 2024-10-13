#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ALPHABET_SIZE 26
#define ALPHABET "abcdefghijklmnopqrstuvwxyz"
typedef struct val val;
struct val {
	int no_letters;
	char *cuv;
};

void free_val(void *aux)
{
	if (!aux)
		return;
	val *value = (val *)aux;
	free(value->cuv);
	value->cuv = NULL;
	free(value);
}

/*Trie lab11*/
typedef struct trie_node_t trie_node_t;

struct trie_node_t {
	/* Value associated with key (set if end_of_word = 1) */
	void *value;
	/*frequency of the word*/
	int freq;
	/* 1 if current node marks the end of a word, 0 otherwise */
	int end_of_word;

	trie_node_t **children;
	int n_children;
};

typedef struct trie_t trie_t;
struct trie_t {
	trie_node_t *root;

	/* Number of keys */
	int size;

	/* Generic Data Structure */
	int data_size;

	/* Trie-Specific, alphabet properties */
	int alphabet_size;
	char *alphabet;

	/* Callback to free value associated with key, should be called when freeing
	 */
	void (*free_value_cb)(void *aux);

	/* Optional - number of nodes, useful to test correctness */
	int n_nodes;
};

trie_node_t *find_smallest_subtrie(trie_node_t *node);
trie_node_t *trie_create_node(trie_t *trie)
{
	trie_node_t *node = calloc(1, sizeof(*node));
	node->children = calloc(trie->alphabet_size, sizeof(*node->children));
	trie->n_nodes++;
	return node;
	// TODO
}

// initialize trie
trie_t *trie_create(int data_size, int alphabet_size, char *alphabet,
					void (*free_value_cb)(void *)
									 )
{
	trie_t *trie = malloc(sizeof(*trie));
	trie->size = 0;
	trie->data_size = data_size;
	trie->alphabet_size = alphabet_size;
	trie->n_nodes = 0;
	trie->alphabet = malloc(sizeof(*trie->alphabet) * trie->alphabet_size);
	memmove(trie->alphabet, alphabet, trie->alphabet_size);
	trie->root = trie_create_node(trie);
	trie->free_value_cb = free_value_cb;
	return trie;
	// TOD0
}

// insert a node in trie
void insert(trie_t *trie, trie_node_t *node, char *key, void *value)
{
	if (key[0] == '\0') {
		if (node->end_of_word == 1) {
			node->freq++;
			free(((val *)value)->cuv);
			return;
		}
		node->value = malloc(trie->data_size);
		// *(int*)(node->value) = *(int*)value;
		memcpy(node->value, value, trie->data_size);
		node->freq++;
		node->end_of_word = 1;
		trie->size++;
		return;
	}
	trie_node_t *next_node = node->children[key[0] - 'a'];
	if (!next_node) {
		node->children[key[0] - 'a'] = trie_create_node(trie);
		node->n_children++;
	}
	insert(trie, node->children[key[0] - 'a'], key + 1, value);
}

void trie_insert(trie_t *trie, char *key, void *value)
{
	if (trie->root->children[key[0] - 'a'] == NULL) {
		trie->root->children[key[0] - 'a'] = trie_create_node(trie);
		trie->root->n_children++;
	}
	trie_node_t *next_node = trie->root->children[key[0] - 'a'];

	insert(trie, next_node, key + 1, value);
	// TODO
}

void *search(char *key, trie_node_t *node)
{
	if (strlen(key) == 0 && node->end_of_word == 1)
		return node->value;
	trie_node_t *next_node = node->children[key[0] - 'a'];
	if (!next_node)
		return NULL;

	return search(key + 1, next_node);
}

void *trie_search(trie_t *trie, char *key)
{
	if (strlen(key) == 0) {
		void *aux = malloc(sizeof(int *));
		*(int *)aux = -1;
		return aux;
	}
	if (trie->root->children[key[0] - 'a'] == NULL)
		return NULL;
	else
		return search(key + 1, trie->root->children[key[0] - 'a']);
	// TODO
}

// remove a node
int remove_node(trie_t *trie, trie_node_t *node, char *key)
{
	if (strlen(key) == 0) {
		trie->free_value_cb(node->value);
		if (node->end_of_word == 1) {
			node->end_of_word = 0;
			if (node->n_children > 0)
				return 0;
			else
				return 1;
		}

		return 0;
}

trie_node_t *next_node = node->children[key[0] - 'a'];
	if (next_node && remove_node(trie, next_node, key + 1) == 1) {
		free(next_node->children);
		free(next_node);
		node->n_children--;
		trie->n_nodes--;
		node->children[key[0] - 'a'] = NULL;
		if (node->n_children == 0 && node->end_of_word == 0)
			return 1;
		return 0;
	}
	return 0;
}

void trie_remove(trie_t *trie, char *key)
{
	trie_node_t *next_node = trie->root->children[key[0] - 'a'];
	if (next_node && remove_node(trie, next_node, key + 1) == 1) {
		free(next_node->children);
		free(next_node);
		trie->n_nodes--;
		trie->root->n_children--;
		trie->root->children[key[0] - 'a'] = NULL;
	}
	// TODO
}

// free a node from trie
void trie_free_nod(trie_t *trie, trie_node_t *node)
{
	if (!node)
		return;
	if (node->end_of_word == 1 && node->value) {
		trie->free_value_cb(node->value);
		node->value = NULL;
	}
	for (int i = 0; i < trie->alphabet_size; i++) {
		if (!node->children[i])
			continue;
		trie_free_nod(trie, node->children[i]);
		node->n_children--;
		node->children[i] = NULL;
	}

	free(node->children);
	node->children = NULL;
	free(node);
	trie->n_nodes--;
}

// free the trie
void trie_free(trie_t **ptrie)
{
	trie_free_nod(*ptrie, (*ptrie)->root);
	free((*ptrie)->alphabet);
	free(*ptrie);
	// TODO
}

/*end of trie lab11*/

// insert a word in trie
// this function was necessary to adapt the functios implemented in lab11
// to what this program needs, initializing the val structure
void insertf(trie_t *trie, char *word)
{
	val *value = malloc(sizeof(val));
	value->no_letters = strlen(word);
	value->cuv = malloc(sizeof(char) * (value->no_letters + 1));
	memcpy(value->cuv, word, value->no_letters + 1);
	insert(trie, trie->root, word, value);
	free(value);
}

// insert all words from file
void load(trie_t *trie, char *file_name)
{
	FILE *file;
	file = fopen(file_name, "r");
	// read from file
	if (!file) {
		printf("Failed to open file");
		return;
	}
	char word[50];
	while (fscanf(file, "%s", word) == 1)
		insertf(trie, word);
	fclose(file);
}

// a recursive function that dispays the words inserted in the trie which
// differ by "changes" number of letters from the string "word" given
void autoccorect_node(trie_t *trie, trie_node_t *node, char *word, int changes)
{
	if (changes < 0)
		return;
	if (word[0] == '\0') {
		if (node->end_of_word == 0)
			return;
		printf("%s\n", ((val *)node->value)->cuv);
		return;
	}
	for (int i = 0; i < trie->alphabet_size; i++) {
		if (!node->children[i])
			continue;
		if (i == word[0] - 'a')
			autoccorect_node(trie, node->children[i], word + 1, changes);
		else
			autoccorect_node(trie, node->children[i], word + 1, changes - 1);
	}
}

// finds the smallest lexicographic word with the given prefix
int autocomplete1(trie_t *trie, trie_node_t *node, char *pref)
{
	if (!node)
		return 0;
	if (pref[0] != '\0')
		return autocomplete1(trie, node->children[pref[0] - 'a'], pref + 1);
	if (node->end_of_word == 1) {
		printf("%s\n", ((val *)node->value)->cuv);
		return 1;
	}
	for (int i = 0; i < trie->alphabet_size; i++)
		if (autocomplete1(trie, node->children[i], pref) == 1)
			return 1;
	return 0;
}

// finds the shortest word with the given prefix
int autocomplete2(trie_t *trie, trie_node_t *node, char *pref)
{
	if (!node)
		return 0;
	if (pref[0] != '\0')
		return autocomplete2(trie, node->children[pref[0] - 'a'], pref + 1);
	if (node->end_of_word == 1 && pref[0] == '\0') {
		printf("%s\n", ((val *)node->value)->cuv);
		return 1;
	}
	return autocomplete2(trie, find_smallest_subtrie(node), pref);
}

// finds the smallest subtrie of the given node
trie_node_t *find_smallest_subtrie(trie_node_t *node)
{
	if (!node)
		return NULL;
	if (node->end_of_word)
		return node;
	trie_node_t *smallest_child = NULL;
	int smallest_size = __INT_MAX__;
	for (int i = 0; i < 26; i++) {
		trie_node_t *current_child = find_smallest_subtrie(node->children[i]);
		if (current_child && current_child->end_of_word &&
			((val *)current_child->value)->no_letters < smallest_size
			 ) {
			smallest_size = ((val *)current_child->value)->no_letters;
			smallest_child = current_child;
		}
	}
	return smallest_child;
}

// finds the most frequent subtrie of the given node
trie_node_t *mostfr(trie_node_t *node)
{
	if (!node)
		return NULL;
	trie_node_t *smallest_child = NULL;
	int biggestfr = -1;
	for (int i = 0; i < 26; i++) {
		trie_node_t *current_child = mostfr(node->children[i]);
		if (current_child && current_child->end_of_word &&
			current_child->freq > biggestfr
			 ) {
			biggestfr = current_child->freq;
			smallest_child = current_child;
		}
	}
	if (node->end_of_word && node->freq >= biggestfr) {
		smallest_child = node;
		biggestfr = node->freq;
	}
	return smallest_child;
}

// finds the most frequently used word with the given prefix
int autocomplete3(trie_t *trie, trie_node_t *node, char *pref)
{
	if (!node)
		return 0;
	if (pref[0] != '\0')
		return autocomplete3(trie, node->children[pref[0] - 'a'], pref + 1);
	trie_node_t *found = mostfr(node);
	if (found) {
		printf("%s\n", ((val *)found->value)->cuv);
		return 1;
	}
	return 0;
}

// depending on the variable "no_c", it is decided which autocomplete
// function to be called
void autocomplete(trie_t *trie, trie_node_t *node, char *pref, int no_c)
{
	if (no_c == 1) {
		if (autocomplete1(trie, node, pref) == 0)
			printf("No words found\n");
	} else if (no_c == 2) {
		if (autocomplete2(trie, node, pref) == 0)
			printf("No words found\n");
	} else if (no_c == 3) {
		if (autocomplete3(trie, node, pref) == 0)
			printf("No words found\n");
	} else if (no_c == 0) {
		autocomplete(trie, node, pref, 1);
		autocomplete(trie, node, pref, 2);
		autocomplete(trie, node, pref, 3);
	}
}

int main(void)
{
	char command[20], word[50], filename[50], pref[50];
	int k;
	trie_t *trie = trie_create(sizeof(val), ALPHABET_SIZE, ALPHABET, free_val);
	while (1) {
		scanf("%s", command);
		if (strncmp(command, "INSERT", 6) == 0) {
			scanf("%s", word);
			insertf(trie, word);
		} else if (strncmp(command, "LOAD", 4) == 0) {
			scanf("%s", filename);
			load(trie, filename);
		} else if (strncmp(command, "REMOVE", 6) == 0) {
			scanf("%s", word);
			trie_remove(trie, word);
		} else if (strncmp(command, "AUTOCORRECT", 11) == 0) {
			scanf("%s", word);
			scanf("%d", &k);
			autoccorect_node(trie, trie->root, word, k);
		} else if (strncmp(command, "AUTOCOMPLETE", 12) == 0) {
			scanf("%s", pref);
			scanf("%d", &k);
			autocomplete(trie, trie->root, pref, k);
		} else if (strncmp(command, "EXIT", 4) == 0) {
			trie_free(&trie);
			break;
		}
	}
	return 0;
}
