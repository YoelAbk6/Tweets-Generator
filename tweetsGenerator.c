#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#define MAX_WORDS_IN_SENTENCE_GENERATION 20
#define MAX_WORD_LENGTH 100
#define MAX_SENTENCE_LENGTH 1000

typedef struct WordStruct {
    char * word;
    struct WordProbability * prob_list;
    int total_occ;
    int unique_duos;
    int full_stop; // 1 = true, 0 = false
}
        WordStruct;

typedef struct WordProbability {
    struct WordStruct * word_struct_ptr;
    int imm_occ;
}
        WordProbability;

/************ LINKED LIST ************/
typedef struct Node {
    WordStruct * data;
    struct Node * next;
}
        Node;

typedef struct LinkList {
    Node * first;
    Node * last;
    int size;
}
        LinkList;

/**
 * Add data to new node at the end of the given link list.
 * @param link_list Link list to add data to
 * @param data pointer to dynamically allocated data
 * @return 0 on success, 1 otherwise
 */
int add(LinkList * link_list, WordStruct * data) {
    Node * new_node = malloc(sizeof(Node));
    if (new_node == NULL) {
        return 1;
    }
    * new_node = (Node) {
            data,
            NULL
    };
    if (link_list -> first == NULL) {
        link_list -> first = new_node;
        link_list -> last = new_node;
    } else {
        link_list -> last -> next = new_node;
        link_list -> last = new_node;
    }

    link_list -> size++;
    return 0;
}
/*************************************/

/**
 * Get random number between 0 and max_number [0, max_number).
 * @param max_number
 * @return Random number
 */
int get_random_number(int max_number) {
    int divisor = RAND_MAX / (max_number);
    int ret_val;
    do {
        ret_val = rand() / divisor;
    } while (ret_val > max_number - 1);
    return ret_val;
}

/**
 * Choose randomly the next word from the given dictionary, drawn uniformly.
 * The function won't return a word that end's in full stop '.' (Nekuda).
 * @param dictionary Dictionary to choose a word from
 * @return WordStruct of the chosen word
 */
WordStruct * get_first_random_word(LinkList * dictionary) {
    Node * temp;
    int rand_num;
    while (1) {
        rand_num = get_random_number(dictionary -> size);
        temp = dictionary -> first;
        for (int i = 0; i < rand_num; i++, temp = temp -> next);
        if (temp -> data -> prob_list) //If not ending with a full stop
            return temp -> data;
    }
}

/**
 * Choose randomly the next word. Depend on its occurrence frequency
 * in word_struct_ptr->WordProbability.
 * @param word_struct_ptr WordStruct to choose from
 * @return WordStruct of the chosen word
 */
WordStruct * get_next_random_word(WordStruct * word_struct_ptr) {
    int cumulative_sum = 0;
    int prob_list_size = word_struct_ptr -> unique_duos;
    int rand_num = get_random_number(word_struct_ptr->total_occ);
    for (int i = 0; i < prob_list_size; i++) {
        cumulative_sum += word_struct_ptr -> prob_list[i].imm_occ;
        if (rand_num < cumulative_sum)
            return word_struct_ptr -> prob_list[i].word_struct_ptr;
    }
    return NULL; //We will not reach here because prob_list_size will always be at least 1
}

/**
 * Receive dictionary, generate and print to stdout random sentence out of it.
 * The sentence most have at least 2 words in it.
 * @param dictionary Dictionary to use
 * @return Amount of words in printed sentence
 */
int generate_sentence(LinkList * dictionary) {
    WordStruct * first_word = get_first_random_word(dictionary);
    WordStruct * curr_word = first_word;
    int word_number = 1;
    while (word_number < MAX_WORDS_IN_SENTENCE_GENERATION && curr_word -> prob_list) {
        printf("%s ", curr_word -> word);
        curr_word = get_next_random_word(curr_word);
        word_number++;
    }
    //if the last word doesn't end with a \n, adds it
    (curr_word -> word[strlen(curr_word -> word) - 1] == '\n') ?
    printf("%s", curr_word -> word): printf("%s\n", curr_word -> word);
    return word_number;
}

/**
 * Gets 2 WordStructs. If second_word in first_word's prob_list,
 * update the existing probability value.
 * Otherwise, add the second word to the prob_list of the first word.
 * @param first_word
 * @param second_word
 * @return 0 if already in list, 1 otherwise.
 */
int add_word_to_probability_list(WordStruct * first_word, WordStruct * second_word) {
    if (!first_word || first_word -> full_stop)
        return 1;
    for (int i = 0; i < first_word -> unique_duos; i++) {
        if (first_word -> prob_list[i].word_struct_ptr == second_word) {
            first_word -> prob_list[i].imm_occ++;
            return 0;
        }
    }
    WordProbability * tmp = realloc(first_word -> prob_list, sizeof(WordProbability) * (first_word -> unique_duos + 1));
    if (!tmp) {
        fprintf(stdout, "Allocation failure: couldn't reallocate the prob_list\n");
        exit(EXIT_FAILURE);
    }
    first_word -> prob_list = tmp;
    first_word -> prob_list[first_word -> unique_duos].word_struct_ptr = second_word;
    first_word -> prob_list[first_word -> unique_duos].imm_occ = 1;
    first_word -> unique_duos++;
    return 1;
}

/**
 * Gets a word and the dictionary.
 * Will allocate a new wordStruct if the word doesn't exits already in the dictionary
 * @param word The word that we want to add
 * @param dictionary The dictionary we want to add to
 * @return The word WordStruct*, existing or new.
 */
WordStruct * add_word_to_dictionary(char * word, LinkList * dictionary) {
    Node * curr = dictionary -> first;
    while (curr && strcmp(curr -> data -> word, word) != 0) {
        curr = curr -> next;
    }
    if (!curr) { //If it's a new word
        WordStruct * newWS = malloc(sizeof(WordStruct));
        if (!newWS) {
            fprintf(stdout, "Allocation failure: failed to allocate WordStruct\n");
            exit(EXIT_FAILURE);
        }
        newWS -> word = malloc(strlen(word) + 1);
        if (!newWS -> word) {
            fprintf(stdout, "Allocation failure: failed to allocate char pointer\n");
            exit(EXIT_FAILURE);
        }
        strcpy(newWS -> word, word);
        newWS -> total_occ = 1;
        newWS -> unique_duos = 0;
        newWS -> prob_list = NULL;
        if (word[strlen(word) - 1] == '.' || word[strlen(word) - 1] == '\n') //Ends with a full stop
            newWS -> full_stop = 1;
        else
            newWS -> full_stop = 0;
        if(add(dictionary, newWS) == 1){
            fprintf(stdout, "Allocation failure: failed to allocate new node\n");
            exit(EXIT_FAILURE);
        }
        return newWS;
    }
    curr -> data -> total_occ++;
    return curr -> data;
}

/**
 * Read word from the given file. Add every unique word to the dictionary.
 * Also, at every iteration, update the prob_list of the previous word with
 * the value of the current word.
 * @param fp File pointer
 * @param words_to_read Number of words to read from file.
 *                      If value is bigger than the file's word count,
 *                      or if words_to_read == -1 than read entire file.
 * @param dictionary Empty dictionary to fill
 */
void fill_dictionary(FILE * fp, int words_to_read, LinkList * dictionary) {
    char buffer[MAX_SENTENCE_LENGTH], * token;
    WordStruct * first_word = NULL, * second_word = NULL;
    while (fgets(buffer, MAX_SENTENCE_LENGTH, fp) && words_to_read != 0) {
        token = strtok(buffer, " ");
        while (token != NULL && words_to_read != 0) {
            words_to_read--;
            second_word = add_word_to_dictionary(token, dictionary);
            add_word_to_probability_list(first_word, second_word);
            first_word = second_word;
            token = strtok(NULL, " ");
        }
    }
}

/**
 * Free the given dictionary and all of it's content from memory.
 * @param dictionary Dictionary to free
 */
void free_dictionary(LinkList * dictionary) {
    Node * curr = NULL;
    while (dictionary -> first) {
        curr = dictionary -> first;
        dictionary -> first = dictionary -> first -> next;
        free(curr -> data -> word);
        free(curr -> data -> prob_list);
        free(curr -> data);
        free(curr);
    }
    free(dictionary);
}

/**
 * Allocates and initialize the dictionary.
 * @return a pointer to the dictionary if malloc succeeded and NULL if not.
 */
LinkList * initialize_dictionary() {
    LinkList * dictionary = malloc(sizeof(LinkList));
    if (dictionary) {
        dictionary -> first = NULL;
        dictionary -> last = NULL;
        dictionary -> size = 0;
        return dictionary;
    }
    return NULL;
}

/**
 * @param argc
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 *             3) Path to file
 *             4) Optional - Number of words to read
 */
int main(int argc, char * argv[]) {
    if (argc < 4 || argc > 5) {
        fprintf(stdout, "Usage: tweetsGenerator seed nsentences path_to_file [nwords]\n");
        exit(EXIT_FAILURE);
    }
    long seed = strtol(argv[1], NULL, 10);
    srand(seed);
    int sentences_to_generate = (int) strtol(argv[2], NULL, 10);
    FILE * fp = fopen(argv[3], "r");
    if (!fp) {
        fprintf(stdout, "Error: could not open file\n");
        exit(EXIT_FAILURE);
    }
    int words_to_read;
    if (argc == 5)
        words_to_read = (int) strtol(argv[4], NULL, 10);
    else
        words_to_read = -1; // We should read the whole file

    LinkList * dictionary = initialize_dictionary();
    if (!dictionary) {
        fprintf(stdout, "Allocation failure: couldn't allocate the dictionary\n");
        exit(EXIT_FAILURE);
    }
    fill_dictionary(fp, words_to_read, dictionary);
    for (int i = 0; i < sentences_to_generate; i++) {
        printf("Tweet %d: ", i + 1);
        generate_sentence(dictionary);
    }
    fclose(fp);
    free_dictionary(dictionary);
    return 0;
}
