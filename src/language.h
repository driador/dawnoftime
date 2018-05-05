/**************************************************************************/
// language.h - dawn language system header
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#ifndef LANGUAGE_H
#define LANGUAGE_H

struct wordmapping_data {
	wordmapping_data *next;
	char *from;
	char *to;
};

class language_data;

class languagetree_data{
public:
	languagetree_data(languagetree_data *theparent, char *characters, char *data);

	void add(languagetree_data *theparent, char *characters, char *data);
	int find(const char *characters, languagetree_data **tail);
	languagetree_data * find_node_with_character(char lookfor);
	void dump_tree(int depth);
	void add_subtree_to_wordlist(int depth, char *workingbuf, language_data *ld);
	char *stored_word;		// if we get this far in the chain, then the word stored here is our replacement

	~languagetree_data(); // destructor

private:
	char character;			// the matching character
	languagetree_data * across; // next possible character 
	languagetree_data * down;// if we have a matching character
	languagetree_data * parent;		// a pointer to the node containing previous character in the word matched
};


class language_data { // definition of a language 
public:
	char *name;
	int unique_id; // used for internally representing the language on scrolls
	char *skillname; // doesn't have to be different than straight name
	char *commandname; // the command used to switch to the language
	int gsn; // global skill number

public:
	void initialise_tree();
	void reinitialise_tree(); // used after a word has been deleted from the wordlist
	void dump_tree();
	void add_wordmap_to_tree(char *from, char *to);
	int find(const char *characters, languagetree_data **tail);	
	
	languagetree_data *tree[256]; // the tree is an array, with pointers based on the first character	
public:
	language_data *next;

	wordmapping_data *words;
	void words_remove_unsupported_basewords(); // removes any non alphanum single character words
	void words_deallocate_list(); // deallocate the list used to load in the words
	void words_recreate_list();
	void words_add_word_mapping(const char *wordfrom, const char *wordto);
	void words_initialise_words_last(); // initialises 'words_last' after a GIOLOAD
	int flags;

private:
	wordmapping_data *words_last; // last in the linked list of words



};

#endif // LANGUAGE_H

/**************************************************************************/
/**************************************************************************/
