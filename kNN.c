/* Copyright (c) 2023, Alexia Enache */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

/* Structure definitions */

typedef struct node node_t;
struct node {
    node_t *left;
    node_t *right;
    long   *arr;    
};

typedef struct {
    size_t  dim;
    node_t *root;
} tree_t;

/* Internal node functions */

node_t *node_create(const long *arr, const size_t arr_size);
void node_free(node_t **node_pp);

/* Internal tree functions */

tree_t *tree_create(const size_t dim);
void tree_free_helper(node_t *root);
void tree_free(tree_t **tree_pp);

node_t *point_insert_helper(const tree_t *tree,
                            node_t *root,
                            node_t *node,
                            const size_t level);
node_t *point_insert(tree_t *tree, const long *arr);

void tree_nearest_neighbour_helper(const tree_t *tree,
                                      const node_t *node,
                                      const long   *target,
                                      double       *best_dist,
                                      node_t      **best_nodes,
                                      size_t       *best_nmemb);

/* Commands */

tree_t *tree_load_from_file(const char *filename);

node_t **tree_nearest_neighbour(const tree_t *tree, const long *arr);

/* Misc functions */

size_t parse_line(char *line, char **words);

double distance(const long *p1, const long *p2, size_t k);

void   arr_print_data(const long *arr, const size_t size);

/* Debug functions */

void dbg_arr_print_data(const long *arr, const size_t size);
void dbg_tree_print_helper(const node_t *root, const size_t size);
void dbg_tree_print(const tree_t *tree);

/* Implementations */

void dbg_tree_print_helper(const node_t *root, const size_t size)
{
    if (!root)
        return;

    dbg_arr_print_data(root->arr, size);

    dbg_tree_print_helper(root->left,  size);
    dbg_tree_print_helper(root->right, size);
}

void dbg_tree_print(const tree_t *tree)
{
    if (!tree)
        return;
    
    dbg_tree_print_helper(tree->root, tree->dim);
}

void dbg_arr_print_data(const long *arr, const size_t size)
{
    printf("DEBUG print for arr %p\n", (void *) arr);

    for (size_t d = 0; d < size; ++d) {
        printf("node->arr[%zu] = %ld\n", d, arr[d]);
    }
}

void arr_print_data(const long *arr, const size_t size)
{
    for (size_t d = 0; d < size; ++d) {
        printf("%ld ", arr[d]);
    }
    printf("\n");
}

node_t *node_create(const long *arr, const size_t arr_size)
{
    node_t *node = calloc(1, sizeof(*node));

    if (!node)
        return NULL;

    node->arr = calloc(arr_size, sizeof(*arr));

    if (!node->arr) {
        free(node);
        return NULL;
    }

    memcpy(node->arr, arr, arr_size * sizeof(*arr));

    return node;
}

void node_free(node_t **node_pp)
{
    if (!node_pp)
        return;
    
    free((*node_pp)->arr);
    free(*node_pp);
    *node_pp = NULL;
}

tree_t *tree_create(const size_t dim)
{
    tree_t *tree = calloc(1, sizeof(*tree));

    if (!tree)
        return NULL;
    
    tree->dim = dim;

    return tree;
}

void tree_free_helper(node_t *root)
{
    if (!root)
        return;
    
    if (root->left)
        tree_free_helper(root->left);
    
    root->left = NULL;
    
    if (root->right)
        tree_free_helper(root->right);
    
    root->right = NULL;

    node_free(&root);
}

void tree_free(tree_t **tree_pp)
{
    if (!*tree_pp)
        return;
    
    tree_free_helper((*tree_pp)->root);
    free(*tree_pp);
    *tree_pp = NULL;
}

node_t *point_insert_helper(const tree_t *tree,
                            node_t *root,
                            node_t *node,
                            const size_t level)
{
    if (!root || !node) {
        if (!root)
            fprintf(stderr,
                    "Error: Empty <tree> in point_insert_helper()!\n");
        if (!node)
            fprintf(stderr,
                    "Error: Empty <root> in point_insert_helper()!\n");
        return NULL;
    }

    if (node->arr[level] < root->arr[level]) {
        if (root->left)
            return point_insert_helper(tree,
                                       root->left,
                                       node,
                                       (level + 1) % tree->dim);
        root->left = node;
    } else {
        if (root->right)
            return point_insert_helper(tree,
                                       root->right,
                                       node,
                                       (level + 1) % tree->dim);
        root->right = node;
    }

    return node;
}

node_t *point_insert(tree_t *tree, const long *arr)
{
    if (!tree) {
        fprintf(stderr, "Error: Empty <tree> in point_insert()!\n");
        return NULL;
    }

    node_t *node = node_create(arr, tree->dim);

    if (!node) {
        perror("node_create() failed");
        return NULL;
    }

    // Base case: Adding the first node of a tree
    if (!tree->root)
        return (tree->root = node);

    node_t *ret = point_insert_helper(tree, tree->root, node, 0);

    if (!ret) {
        fprintf(stderr, "Error: point_insert_helper() failed!\n");
        node_free(&node);
        return NULL;
    }

    return ret;
}

tree_t *tree_load_from_file(const char *filename)
{
    FILE *fp = fopen(filename, "r");

    if (!fp)
        return NULL;

    tree_t *tree = NULL;
    size_t  n, k;

    if (fscanf(fp, "%zu %zu", &n, &k) != 2 || n > 10001) {
        fprintf(stderr,
                "Error: Failed to read <n> and <k> from %s!\n", filename);
        fclose(fp);

        return NULL;
    }

    if (!(tree = tree_create(k))) {
        fprintf(stderr,
                "Error: Failed to create k&d tree with k = %zu!\n", k);
        fclose(fp);

        return NULL;
    }
        
    
    long *arr_aux = calloc(k, sizeof(*arr_aux));

    if (!arr_aux) {
        fprintf(stderr,
                "Error: Failed to allocate %zu bytes of memory for <arr>\n",
                k * sizeof(*arr_aux));
        fclose(fp);

        return NULL;
    }

    for (size_t i = 0; i < n; ++i) {
        for (size_t d = 0; d < k; ++d) {
            if (fscanf(fp, "%ld", &arr_aux[d]) != 1) {
                fprintf(stderr,
                        "Error: Failed to read "
                        "dimension %zu of node %zu from %s!\n",
                        d, i, filename);
                free(arr_aux);
                tree_free(&tree);
                fclose(fp);

                return NULL;
            }
        }
        if (!point_insert(tree, arr_aux)) {
            fprintf(stderr,
                    "Error: Failed to insert node to tree! DEBUG n = %zu, k = %zu\n",
                    n, k);

            dbg_arr_print_data(arr_aux, tree->dim);

            free(arr_aux);
            tree_free(&tree);
            fclose(fp);

            return NULL;
        }
    }

    free(arr_aux);
    fclose(fp);

    return tree;
}

double distance(const long *p1, const long *p2, size_t k)
{
    double dist = 0;
    double diff = 0;

    for (size_t i = 0; i < k; i++) {
        diff  = p1[i] - p2[i];
        dist += diff * diff;
    }

    return sqrt(dist);
}

void tree_nearest_neighbour_helper(const tree_t *tree,
                                      const node_t *node,
                                      const long   *target,
                                      double       *best_dist,
                                      node_t      **best_nodes,
                                      size_t       *best_nmemb)
{
    if (node == NULL)
        return;

    double dist = distance(node->arr, target, tree->dim);
    double diff = dist - *best_dist;
    if(fabs(diff) < 0.001) {
        best_nodes[(*best_nmemb)++]   = (node_t *) node;
    } else if (dist < *best_dist) {
        *best_dist    = dist;
        *best_nmemb   = 1;
        best_nodes[0] = (node_t *) node;
    }
    
    tree_nearest_neighbour_helper(tree, node->left, target, best_dist, best_nodes, best_nmemb);
    tree_nearest_neighbour_helper(tree, node->right, target, best_dist, best_nodes, best_nmemb);
    
}

void sort_vec(node_t **vec, size_t count) {
    if(count < 2)
    return;
    for(size_t i = 0; i < count - 1; i++) {
        for(size_t j = i + 1; j < count; j++) {
            if(vec[i]->arr[0] > vec[j]->arr[0]) {
                node_t* temp = vec[i];
                vec[i] = vec[j];
                vec[j] = temp;
            } else if (vec[i]->arr[0] == vec[j]->arr[0] && vec[i]->arr[1] > vec[j]->arr[1]) {
                node_t* temp = vec[i];
                vec[i] = vec[j];
                vec[j] = temp;
            }
        }
    }

}

node_t **tree_nearest_neighbour(const tree_t *tree, const long *target) {
    if (!tree || !tree->root)
        return NULL;

    double  best_dist  = distance(tree->root->arr, target, tree->dim);
    size_t  best_nmemb = 0;

    node_t **best_nodes = malloc(BUFSIZ);

    best_nodes[0]      = tree->root;

    tree_nearest_neighbour_helper(
        tree, tree->root, target, &best_dist, best_nodes, &best_nmemb);
    sort_vec(best_nodes, best_nmemb);
    for (size_t i = 0; i < best_nmemb; ++i) {
        arr_print_data(best_nodes[i]->arr, tree->dim);
    }

    return best_nodes;
}

int is_node(const node_t *node, node_t **result, size_t count) {
    for(size_t i = 0; i < count; i++) {
        if(result[i] == node) {
            return 1;
        }
    }
    return 0;
}

void tree_range_search_helper(const tree_t * tree, const node_t *node, const long *range, size_t dim, int axis, node_t **result, size_t *count) {
    if (node == NULL)
        return;
    
    if (node->arr[dim] >= range[axis] && node->arr[dim] <= range[axis + 1]) {
        // Check if the node is within the range in the current dimension
        if (dim == (tree->dim - 1)) {
            // If we reached the last dimension, add the node to the result array
            if(!is_node(node, result, *count)) {
                result[*count] = (node_t *)node;
                (*count)++;
            }
        } else {
            // Recursively search in the next dimension
            tree_range_search_helper(tree, node, range, dim + 1, axis + 2, result, count);
        }
}
    tree_range_search_helper(tree, node->left, range, 0, 0, result, count);
    tree_range_search_helper(tree, node->right, range, 0, 0, result, count);
}

node_t **tree_range_search(const tree_t *tree, const long *range,
      size_t *result_count)
{
    if (!tree || !tree->root)
        return NULL;

    size_t max_nodes = 100; // Maximum number of nodes to be returned
    node_t **result = (node_t **)malloc(sizeof(node_t *) * max_nodes);
    *result_count = 0;

    tree_range_search_helper(tree, tree->root, range, 0, 0, result,
                 result_count);
    sort_vec(result, *result_count);
    for (size_t i = 0; i < *result_count; ++i)
        arr_print_data(result[i]->arr, tree->dim);
    return result;
}

size_t parse_line(char *line, char **words)
{
    size_t count = 0;
    for (char *p = strtok(line, " \t\n"); p; p = strtok(NULL, " \t\n"))
        words[count++] = p;
    return count;
}

int main(void)
{
    char    line[BUFSIZ] = { 0 };
    char  *words[BUFSIZ] = { 0 };
    size_t wcount        = 0;

    tree_t *tree         = NULL;  // Should be initialized only ONCE by calling
                                  // "LOAD <filename>"

    for (;;) {
        if (!fgets(line, BUFSIZ, stdin))
            return EXIT_FAILURE;

        wcount = parse_line(line, words);

        if (!wcount)
            continue;  // Empty line

        if (wcount == 2 && !strcmp(words[0], "LOAD")) {
            if (tree) {
                fprintf(stderr,
                        "Error: Tree already initialized! Exiting...\n");
                tree_free(&tree);
                return EXIT_FAILURE;
            }
            tree = tree_load_from_file(words[1]);

            if (!tree) {
                fprintf(stderr, "Error: Failed to load tree from file!\n");
                return EXIT_FAILURE;
            }
        } else if (wcount == 1 && !strcmp(words[0], "EXIT")) {
            tree_free(&tree);
            return EXIT_SUCCESS;
        } else if (tree && wcount == 1 + tree->dim && !strcmp(words[0], "NN")) {
            long arr_aux[tree->dim];

            for (size_t i = 1; i <= tree->dim; ++i)
                arr_aux[i - 1] = atol(words[i]);
            
            node_t **nodes = NULL;

            if (!(nodes = tree_nearest_neighbour(tree, arr_aux))) {
                fprintf(stderr,
                        "Error: Failed to find nearest neighbour of:\n");
                dbg_arr_print_data(arr_aux, tree->dim);
                tree_free(&tree);

                return EXIT_FAILURE;
            } else {
                free(nodes);
            }
        } else if (wcount == tree->dim * 2 + 1 && !strcmp(words[0], "RS")){
            long arr_aux[tree->dim * 2];
            node_t **nodes = NULL;
            size_t *count_nodes = calloc(1, sizeof(*count_nodes));
            for (size_t i = 1; i <= (tree->dim * 2); i++)
                arr_aux[i - 1] = atol(words[i]);
            if (!(nodes = tree_range_search(tree, arr_aux, count_nodes))) {
                fprintf(stderr,
                        "Error: Failed to find nearest neighbour of:\n");
                dbg_arr_print_data(arr_aux, tree->dim);
                tree_free(&tree);

                return EXIT_FAILURE;
            } else {
                free(nodes);
                free(count_nodes);
            }
        } else if (wcount == 1 && tree && !strcmp(words[0], "DEBUG")) {
                dbg_tree_print(tree);
        } else {
            fprintf(stderr,
                    "Warning: Invalid command <%s>, try again!\n",
           words[0]);
        }
    }
    return EXIT_FAILURE;
}
