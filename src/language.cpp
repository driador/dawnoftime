/**************************************************************************/
// language.cpp - dawn language system, completely rewritten 16 march 2003
/***************************************************************************
 * The Dawn of Time v1.69s.beta6 (c)1997-2010 Kalahn                       *
 * >> A number of people have contributed to the Dawn codebase, with the   *
 *    majority of code written by Kalahn - www.dawnoftime.org              *
 * >> To use this source code, you must fully comply with the dawn license *
 *    in licenses.txt... In particular, you may not remove this copyright  *
 *    notice.                                                              *
 **************************************************************************/
#include "include.h"
#include "language.h"

language_data *languages;

/**************************************************************************/
/*
// Language lookups are performed using a tree, which goes 'across' and 'down'  

  The words "Attention, Attentive and Attractive" are stored as follows:

   [A]->[B]->[C]->[D]
    .
    T
    .
    T
    .
    E    ->   R
    .         .
    N         A
    .         .
    T         C
    .         .
    I        [T]
    .         .
    O -> V    I
    .    .    .
   [N]  [E]   V
              .
             [E]
    

  Where:
     -> represents the next possible character instead of the one to the left
     .  represents the next character in the string creating a word
    [ ] represents a node with a stored_word set

  The following output demonstrates the structure using the builtin dump features
    ===elven
                      ->  ' '
    a                 ->  'a'
     s                ->  've'
      h               ->  'lith'
     gain             ->  'en'
     nger             ->  'ruth'
     byss             ->  'ia'
      horrence        ->  'deloth'
      ominable        ->  'saur'
      ide             ->  'mar'
      road            ->  'palan'
     xes              ->  'baruk'
     utumn            ->  'yavie'
     we               ->  'gaya'
      akening         ->  'coire'
     ir               ->  'wilya'
     ll               ->  'ilya'
      as              ->  'ai'
      one             ->  'er'
    ...
    w                 ->  'x'
     ay               ->  'pata'
      ll              ->  'ramba'
      nder            ->  'raen'
          er          ->  'randir'
      ter             ->  'nen'
         fall         ->  'lanthir'
       ch             ->  'tir'
      ve              ->  'falma'
     right            ->  'dan'
     hole             ->  'iluve'
      ite             ->  'glos'
     ing              ->  'rama'
       ter            ->  'hrive'
       d              ->  'sul'
        ow            ->  'henneth'
      sdom            ->  'noldo'
      thout           ->  'ar'
      ll              ->  'uva'
        ow            ->  'tasare'
     eapon            ->  'saalah'
      rewolf          ->  'nguar'
      ek              ->  'enquie'
      st              ->  'numen'
     ord              ->  'quetta'
      lf              ->  'draug'
      ods             ->  'taure'

  This structures makes it very efficient to perform lookups, by searching 
  for a letter at a time in the word tree... while also making it possible
  to extend the system dynamically.
  
*/
/**************************************************************************/
// this is a custom GIO function, which makes it so languages flagged
// as a system language aren't saved within the list GIO saves
int language_data_gio_dontsavetest(gio_type *gio_table, int tableIndex,
													  void *data, FILE *fp)
{
	language_data * language;
	language= (language_data *) data;

	if(IS_SET(language->flags, LANGFLAG_SYSTEM_LANGUAGE)){
		// system languages aren't saved into the index
		return true;
	}
	return false;
}

/**************************************************************************/
GIO_START(language_data)
GIO_CUSTOM_DONT_SAVE_RECORD(language_data_gio_dontsavetest)
GIO_STR(name)
GIO_STR(skillname)
GIO_STR(commandname)
GIO_WFLAGH(flags, "langflags ", language_flags)
GIO_FINISH_STRDUP_EMPTY
/**************************************************************************/
GIO_START(wordmapping_data)
GIO_STR(from)
GIO_STR(to)
GIO_FINISH_STRDUP_EMPTY

/**************************************************************************/
void languages_save(char_data *ch, char *argument)
{
	language_data *language;

	log_string("Saving language wordmaps...");
	for(language=languages; language; language=language->next){
		if(IS_SET(language->flags, LANGFLAG_SYSTEM_LANGUAGE)){
			// system languages aren't saved into the index
			continue;
		}

		if(str_cmp(argument, "all")){
			if(!IS_SET(language->flags, LANGFLAG_CHANGED)){
				// only save changed languages
				continue;
			}
		}

		// mark it as saved, so we dont write this flag to disk
		REMOVE_BIT( language->flags, LANGFLAG_CHANGED);

		// create the wordlist for saving, save it, then deallocate the wordlist.
		language->words_recreate_list();

		ch->printlnf("Saving %s language wordmap to %s", language->name, FORMATF(LANGUAGES_DIR "%s.txt", language->name));
		GIOSAVE_LIST(language->words, wordmapping_data, FORMATF(LANGUAGES_DIR "%s.txt", language->name), true);		
		language->words_deallocate_list();
	}
	log_string("Language wordmaps saved.");

	log_string("Saving index of languages...");
	// save our list of languages
	GIOSAVE_LIST(languages, language_data, LANGUAGES_INDEX_FILE, true);

	log_string("Languages index saved."); 
	ch->printlnf("Saving index of languages to %s", LANGUAGES_INDEX_FILE);
	ch->println("Language save completed.");
}
/**************************************************************************/
void do_write_languages(char_data *ch, char *argument)
{
	languages_save(ch, argument);
}

/**************************************************************************/
void languages_create_system_languages()
{
	log_string("Creating system languages:");
	language_data *ld;

	// the languages are created in the oposite order so we have 
	// 'unknown' first in the list of languages

	// language_reverse
	log_string("- language reverse");
	language_reverse=new language_data;
	ld=language_reverse;
	ld->gsn=-1;
	ld->name=str_dup("reverse");
	ld->skillname=str_dup("reverse");
	ld->commandname=str_dup("reverse");
	ld->flags=LANGFLAG_SYSTEM_LANGUAGE|LANGFLAG_NO_LANGUAGE_NAME
							|LANGFLAG_NO_ORDER|LANGFLAG_REVERSE_TEXT;
	ld->words=NULL;
	ld->initialise_tree();
	// deallocate the memory used to load in the words, since they aren't
	// typically used once we have constructed the word tree
	ld->words_deallocate_list();
	ld->next=languages;
	languages=ld;

	// language_alwaysunderstood
	log_string("- language alwaysunderstood");
	language_alwaysunderstood=new language_data;
	ld=language_alwaysunderstood;
	ld->gsn=-1;
	ld->name=str_dup("alwaysunderstood");
	ld->skillname=str_dup("");
	ld->commandname=str_dup("-");
	ld->flags=LANGFLAG_SYSTEM_LANGUAGE|LANGFLAG_NO_LANGUAGE_NAME 
						|LANGFLAG_NO_ORDER|LANGFLAG_NO_COMMAND_ACCESS;
	ld->words=NULL;
	ld->initialise_tree();
	// deallocate the memory used to load in the words, since they aren't
	// typically used once we have constructed the word tree
	ld->words_deallocate_list();
	ld->next=languages;
	languages=ld;

	// language_native
	log_string("- language native");
	language_native=new language_data;
	ld=language_native;
	ld->gsn=-1;
	ld->name=str_dup("native");
	ld->skillname=str_dup("native");
	ld->commandname=str_dup("native");
	ld->flags=LANGFLAG_SYSTEM_LANGUAGE | LANGFLAG_NO_ORDER;
	ld->words=NULL;
	ld->initialise_tree();
	// deallocate the memory used to load in the words, since they aren't
	// typically used once we have constructed the word tree
	ld->words_deallocate_list();
	ld->next=languages;
	languages=ld;

	// language_unknown
	log_string("- language unknown");
	language_unknown=new language_data;
	ld=language_unknown;
	ld->gsn=-1;
	ld->name=str_dup("unknown");	
	ld->skillname=str_dup("");
	ld->commandname=str_dup("-");
	ld->flags=LANGFLAG_SYSTEM_LANGUAGE | LANGFLAG_NO_ORDER | LANGFLAG_NO_COMMAND_ACCESS; 
	ld->words=NULL;	
	ld->initialise_tree();	
	// deallocate the memory used to load in the words, since they aren't
	// typically used once we have constructed the word tree
	ld->words_deallocate_list();
	ld->next=languages;
	languages=ld;

	log_string("System languages created.");
}

/**************************************************************************/
void languages_load_and_initialise()
{
	log_string("Loading Languages...");
	// load our list of languages
	languages=NULL;
	language_data *language;
	GIOLOAD_LIST(languages, language_data, LANGUAGES_INDEX_FILE);

	// remove any language with a name of:
	// "unknown", "native", "alwaysunderstood" or "reverse"
	// at the same time check to ensure there are no spaces or 
	// single quotes in the language names
	language_data *previous_language=NULL;
	for(language=languages; language; language=language->next){
		if(is_exact_name(language->name, "unknown native alwaysunderstood reverse")){
			bugf("Found system language '%s' in %s - removing it from the list.", 
				language->name, LANGUAGES_INDEX_FILE);
			if(previous_language){
				previous_language->next=language->next;
			}else{
				languages=language->next;
			}
		}else{
			previous_language=language;
		}

		if(has_space(language->name)){
			logf("language '%s' has a space in its name - this is not supported, manually fix then restart.",
				language->name);
			exit_error( 1 , "languages_load_and_initialise", "invalid language name - has space");
		}
		if(count_char(language->name, '\'')!=0){
			logf("language '%s' has a single quote character in the name - this is not supported, manually fix then restart.",
				language->name);
			exit_error( 1 , "languages_load_and_initialise", "invalid language name - has single quote");
		}
	}

	// load each individual languages worddata
	for(language=languages; language; language=language->next){
		GIOLOAD_LIST(language->words, wordmapping_data, FORMATF(LANGUAGES_DIR "%s.txt", language->name));
		language->words_remove_unsupported_basewords();
		language->words_initialise_words_last();
		logf("Initialising '%s' lookup tree.", language->name);
		language->initialise_tree();
		// deallocate the memory used to load in the words, since they aren't
		// typically used once we have constructed the word tree
		language->words_deallocate_list();

		// setup the skill name and gsn
		if(IS_NULLSTR(language->skillname)){
			language->skillname=str_dup(language->name);
		}

		language->gsn=-1; // gets initialised later after the skills have been loaded
	}

	// loop thru all languages, printing the wordmaps in each one 
//	wordmapping_data *word;
//    for(language=languages; language; language=language->next){
//		logf("======%10s\n", language->name);
//		for(word=language->words; word; word=word->next){
//			logf("%20s -> %s\n", word->from, word->to);
//		}
//	}
	log_string("Language load complete.");

	// create system languages
	languages_create_system_languages();
}

/**************************************************************************/
void language_init_gsn_and_unique_id()
{	
	language_data *language;

	log_string("Initialising language gsn values");
	
	// load each individual languages worddata
	for(language=languages; language; language=language->next){		
		language->gsn=skill_exact_lookup(language->skillname);

		if(IS_NULLSTR(language->commandname)){
			language->commandname=str_dup(language->name);
		}

		if(IS_NULLSTR(language->skillname)){
			continue;
		}

		if(language->gsn<0){
			bugf("can't find a language skill called '%s' for '%s'", 
				language->skillname, language->name);
		}
	}

	log_string("Initialising language unique ids");
	// unique id's are used by items of type parchment, to record the 
	// language the parchment is written in
	int id=0;
	for(language=languages; language; language=language->next){
		language->unique_id=++id;
	}

}

/**************************************************************************/
// the data stored in 'words' is only used to load and save the wordlist
void language_data::words_deallocate_list()
{
	wordmapping_data *word, *worc_next;
	for(word=words; word; word=worc_next){
		// backup the next pointer
		worc_next=word->next;

		// deallocate the memory used in this node
		free_string(word->from);
		free_string(word->to);
		word->next=NULL;
		free(word);
	}
	words=NULL;
	words_last=NULL;
}
/**************************************************************************/
void language_data::words_initialise_words_last()
{
	wordmapping_data *word;
	words_last=NULL;
	for(word=words; word; word=word->next){
		words_last=word;
	}
}
/**************************************************************************/
void language_data::words_remove_unsupported_basewords()
{
	wordmapping_data *word, *prev=NULL;
	for(word=words; word; word=word->next){
		if(str_len(word->from)<2 && !is_alnum( *(word->from) ) ){
			if(prev){
				prev->next=word->next;
			}else{
				words=word->next;
			}
			logf("words_remove_unsupported_basewords(): removed '%s' -> '%s'", word->from, word->to);				
			// aren't worried about freeing memory here, since this will 
			// only happen when they have invalid characters in the
			// base wordmap - not normal.
			continue;
		}
		prev=word;
	}
}
/**************************************************************************/
// append the word mapping into the chain of words
void language_data::words_add_word_mapping(const char *wordfrom, const char *wordto)
{
	wordmapping_data *node;
	
	// sanity check the input
	assert(!IS_NULLSTR(wordfrom));

	node = (wordmapping_data *)malloc(sizeof(wordmapping_data));
	node->from=str_dup(wordfrom);
	node->to=str_dup(wordto);
	node->next=NULL;
	
	// append to the list of words
	if(words_last){
		words_last->next=node;
		words_last=node;
	}else{
		assert(!words);
		words=node;
		words_last=node;
	}
}
/**************************************************************************/
// the data stored in 'words' is only used to load and save the wordlist
// we are most likely just about to save the language to disk
void language_data::words_recreate_list()
{
	char buf[MSL];
	buf[0]='\0';

	// first confirm there isn't any existing list
	words_deallocate_list();

	// now get the tree to add each word to the table
	for(int i=0; i<256; i++){
		if(tree[i]){
			tree[i]->add_subtree_to_wordlist(0, buf, this);
		}
	}
}

/**************************************************************************/
int language_data::find(const char *characters, languagetree_data **tail)
{
	int length=tree[(unsigned char)*characters]->find(characters, tail);
	if(length==0 && tree[(unsigned char)LOWER(*characters)]){
		length=tree[(unsigned char)LOWER(*characters)]->find(characters, tail);
	}
	return length;
}

/**************************************************************************/
void language_data::add_wordmap_to_tree(char *from, char *to)
{
	//logf("%20s -> %s\n", from, to);
	if(tree[(unsigned char)*from]){
		tree[(unsigned char)*from]->add(NULL, from, to);
	}else{
		tree[(unsigned char)*from]=new languagetree_data(NULL, from, to);
	}

}
/**************************************************************************/
// from a loaded wordlist, create the word tree
void language_data::initialise_tree()
{
	// start with an empty tree
	memset(tree, 0, sizeof(tree)); // start off with an empty tree

	// loop thru all wordmaps, adding them into the tree
	wordmapping_data *word;
	for(word=words; word; word=word->next){
		add_wordmap_to_tree(word->from, word->to);
	}
//	dump_tree();
//	fgetc(stdin);
}

/**************************************************************************/
// reinitialise the tree from a loaded wordlist
// deallocating the existing tree first
// used by langedit delword.
void language_data::reinitialise_tree()
{
	int i;
	// deallocate all nodes coming off the existing tree array
	for(i=0; i<256; i++){
		if(tree[i]){
			delete tree[i];
		}
		tree[i]=NULL;
	}
	// we have an empty tree array, readd everything

	wordmapping_data *word;
	// loop thru all wordmaps, adding them into the tree
	for(word=words; word; word=word->next){
		add_wordmap_to_tree(word->from, word->to);
	}
}

/**************************************************************************/
languagetree_data * languagetree_data::find_node_with_character(char lookfor)
{
	if(!this){ // if we are NULL, return NULL
		return NULL; 
	}

	// check if we hold the matching character, return ourselves
	if(lookfor==character){
		return this;
	}

	// otherwise recursively ask the across node if they match
	return across->find_node_with_character(lookfor);	
}
/**************************************************************************/
languagetree_data::languagetree_data(languagetree_data *theparent, char *characters, char *data)
{
	// this is a new node, fill out the data
	character=LOWER(*characters);

	across=NULL;
	parent=theparent;
	stored_word=NULL;

	// check if there are more characters which follow, if so work recursively
	if(*(characters+1)){
		// because we know this is a new node, we just create more down the chain
		down = new languagetree_data(this, characters+1, data);
	}else{
		// we are a leaf node
		stored_word=str_dup(data);
		down=NULL;
	}
}
/**************************************************************************/
languagetree_data::~languagetree_data()
{
	if(down){
		delete down;
	}
	if(across){
		delete across;
	}
}
/**************************************************************************/
void languagetree_data::add(languagetree_data *theparent, char *characters, char *data)
{
	// - we are adding to something that the start of already exists, 
	// - we first must trace our path down what has already been created, when
	//   it differs from the character sequence we have to start creating
	// - if we reach the end of our characters, but we are in an existing node
	//   we have to populate the stored_word... an example of this would be
	//   if we had the word 'action' already in the tree, then the word 'act'
	//   was added.

	// first confirm the character which our node represents is us
	if(character == LOWER(*characters)){
		// we are the node responsible for storing this data

		// decide if we want to hand it to a downstream node, or this is
		// the end of the characters and we need to update or storage
		if(*(characters+1)){
			// there are more characters to follow

			// first check if we have a downstream
			if(down){
				// downstream exists, chop off the front character and pass it down
				down->add(this, characters+1, data);
			}else{
				// no downstream, use the constructor to create the required subtree
				down = new languagetree_data(this, characters+1, data);
			}
		}else{
			// we are the last in the character sequence, store the data here
			stored_word=str_dup(data);			
		}
	}else{
		// the character sequence isn't actually for us, so find the possible peer and add it
		languagetree_data *peernode=find_node_with_character(*characters);
		if(peernode){
			// we have found a peer, hand the data over to the peer and let it handle its downstream creation
			peernode->add(theparent, characters, data);
		}else{
			// there is no node using this value, so use the constructor to create the chain
			// then we merge the subtree of nodes into our across chain
			peernode=new languagetree_data(theparent, characters, data);

			// chain it into the across chain
			peernode->across=across;
			across=peernode;
		}
	}

}

/**************************************************************************/
// - if we are the matching node for this letter, 
//   check recursively if below us matches the next letter and has a payload.
// - if below matches and has a payload, it will return a non 0 result for 
//   the number of characters lower that have been matched.
//   we then return the resulting number + 1 to above.
// - if it doesn't it we check if we have a payload, if we do, we set the
//   the tail to ourself and return a 1.
// - if we aren't the matching node, check for a matching node across,
//   if none found return a 0, if one is found call that node with our
//   parameters and return its result.
int languagetree_data::find(const char *characters, languagetree_data **tail)
{
	if(!this){ // we don't exist so return 0
		return 0;
	}
	int result;

	// first confirm the character which our node represents is us
	if(character == LOWER(*characters)){
		// we are the node responsible containing this data

		// try passing it downstream if possible
		if(down && *(characters+1)){
			result=down->find(characters+1, tail);
			if(result){
				return result+1; // +1 because we absorbed a character in the process
			}
		}

		// downstream didn't match
		// since we match, check if we have a stored_word payload
		if(!IS_NULLSTR(stored_word)){
			// we have a stored_word
			*tail=this;
			return 1;
		}

		// we don't match for our payload, so return 0, and the upper layers can match
		return 0;

	}else{
		// the character sequence isn't actually for us, so find the possible peer and add it
		languagetree_data *peernode=find_node_with_character(*characters);
		if(peernode){
			// we have found a peer, hand the data over to the peer and return its result
			return peernode->find(characters, tail);
		}else{
			// there is no node using this value, so we return a 0.
			return 0;
		}
	}

}

/**************************************************************************/
// Diagnostic code originally written to confirm the tree creation 
// algorthim was correct... the creation algorithms was actually correct
// first compile but the display algorithm took a few tweaks to get right 
// - goes to show commenting code as you go along is a good thing.
// - Kal, March 02.
void languagetree_data::dump_tree(int depth)
{
	logf("%c", character);
	if(!IS_NULLSTR(stored_word)){
		logf("%*c  ->  '%s'", 15-depth, ' ', stored_word);
	}else if(!down && !across){
		logf("%*c  ->  ''", 15-depth, ' ');
	}

	if(down){
		if(!IS_NULLSTR(stored_word)){
			logf("\n%*c", depth+1, ' ');
		}
		down->dump_tree(depth+1);
	}
	if(across){
		logf("\n%*c", depth, ' ');
		across->dump_tree(depth);
	}

}
/**************************************************************************/
// move thru the subtree, adding all word mappings into the wordlist of the language data
void languagetree_data::add_subtree_to_wordlist(int depth, char *working, language_data *ld)
{
	assertp(ld);
	working[depth]=character;
	working[depth+1]='\0';

	if(!IS_NULLSTR(stored_word)){
		ld->words_add_word_mapping( working, stored_word);
//		logf("%-15s -> '%s'\n", working, stored_word);
	}else if(!down && !across){
		ld->words_add_word_mapping( working, stored_word);
//		logf("%-15s -> '%s'\n", working, stored_word);
	}

	if(down){
		down->add_subtree_to_wordlist(depth+1, working, ld);
	}
	if(across){
		across->add_subtree_to_wordlist(depth, working, ld);
	}
}
/**************************************************************************/
// do a tree dump for each variable
void language_data::dump_tree()
{
	logf("===%s", name);
	for(int i=0; i<256; i++){
		if(tree[i]){
			logf("\n");
			tree[i]->dump_tree(0);
			logf("\n");
		}
	}
}
/**************************************************************************/
// # code used to create the language text files from 
// # the original hardcoded language table.  
// # left here because someone may find it useful when upgrading.
// creates a linked list structure representing the language table
// then save it to disk using GIO
// - Kal, March 03
/*
GIO_PROTOTYPE(language_data)
GIO_PROTOTYPE(wordmapping_data)
void language_convert_from_table()
{
	int i;
	int lang;
	language_data *language;
	language_data *languages=NULL;
	wordmapping_data *word;

	// create the link list structures storing all table based language information 
//	#define MAX_LANGUAGE			(22)
    for(lang = 0; lang < MAX_LANGUAGE; lang++){		
		language=new language_data;
		language->name=str_dup(language_table[lang]);
		language->words=NULL;

		for(i=0; !IS_NULLSTR(trans_table[lang][i].old); i++){
			word = new wordmapping_data;
			word->from=str_dup(trans_table[lang][i].old);
			word->to=str_dup(trans_table[lang][i].nw);
			word->next=language->words;
			language->words=word;
		}

		language->next=languages;
		languages=language;			
	}

	// write the link lists to disk
	GIOSAVE_LIST(languages, language_data, LANGUAGES_INDEX_FILE, true);
	for(language=languages; language; language=language->next){
		logf("======%10s\n", language->name);
		for(word=language->words; word; word=word->next){
	//		logf("%20s -> %s\n", word->from, word->to);
		}
		GIOSAVE_LIST(language->words, wordmapping_data, FORMATF(LANGUAGES_DIR "%s.txt", language->name), true);
	}
}
*/
/**************************************************************************/
language_data *language_exact_lookup(const char *name)
{
	language_data *result;
	
    if(IS_NULLSTR(name)){
        log_string("BUG: language_lookup: was past a NULL string!");
        return NULL;
    }

    // loop thru the list of languages
	for(result=languages; result; result=result->next){
		if(!str_cmp(name,result->name)){
			return result;
		}
	}

	return NULL;
}
/**************************************************************************/
language_data *language_lookup(const char *name)
{
	language_data *result;
	
    if(IS_NULLSTR(name)){
        log_string("BUG: language_lookup: was past a NULL string!");
        return NULL;
    }

	// try an exact match first
	result=language_exact_lookup(name);
	if(result){
		return result;
	}

    // now try a prefix match
	for(result=languages; result; result=result->next){
		if(!str_prefix(name,result->name)){
			return result;
		}
	}

	return NULL;
}
/**************************************************************************/
// will return "unknown" language if it can't find a language with the name requested
language_data *language_safe_lookup(const char *name)
{
	language_data *result=language_lookup(name);

	if(!result){
		result=language_lookup("unknown");
		assertp(result); // there should always be an unknown language
	}	
	return result;
}
/**************************************************************************/
language_data *language_lookup_by_id(int id)
{
	language_data *result;
    // loop thru the list of languages
	for(result=languages; result; result=result->next){
		if(result->unique_id==id){
			return result;
		}
	}

	return NULL;
}
/**************************************************************************/
// will return "unknown" language if it can't find a language with the id requested
language_data *language_safe_lookup_by_id(int id)
{
	language_data *result=language_lookup_by_id(id);

	if(!result){
		result=language_lookup("unknown");
		assertp(result); // there should always be an unknown language
	}	
	return result;
}
/**************************************************************************/
// by Imi - reverses the string
char * strreverse(char * string) {
  int low = 0;
  int high = 0 ;

  char lowc;

  high = ( str_len(string) - 1);

  while(low <= high) {

    lowc = string[low];

    string[low] = string[high];
    string[high] = lowc;

    low++;
    high--;
  }

  return string;

}
/**************************************************************************/
void translate_language(language_data *language, bool display_language, char_data * speaker, 
						char_data * listener, const char *message, char *output)
{
	bool unconditionally_understood=false;
	char buf  [MSL];
	char buf2 [MSL];

	if(GAMESETTING3(GAMESET3_LANGUAGE_NOT_SCRAMBLED)){
		// if language is disabled, just copy everything straight across
		strcpy(output,message);
		return;
	}

	int ss, ls; // skills for the speaker and listener

	if(!language){
		bugf("language unspecified, speaker=%s, message='%s'... setting language to native.", 
			PERS(speaker, NULL), message);
		language=language_native;
	}
	
	if(language->gsn<0){ 
		// if there is no skill, at this stage they get it at 100%
		ss=100;
		ls=100;
	}else{
		// get the skills of each char in the language + 10% 
		ss=get_skill(TRUE_CH(speaker),language->gsn)+10;
		ls=get_skill(TRUE_CH(listener),language->gsn)+10;
	}

	// 100% ability in the language for non controlled mobs
	if (IS_NPC(speaker) && !IS_CONTROLLED(speaker)){
		ss=100;
	}

	// support speaking in a players native language	
	if(language==language_native){
		unconditionally_understood=true;
		language=race_table[listener->race]->language;
		if(!language){
			language=language_alwaysunderstood;
			unconditionally_understood=true;
		}
	}

	if(// checks for straight translation
			( // holyspeech, unless disabled for the given language
				!IS_SET(language->flags,LANGFLAG_NO_HOLYSPEECH)
				&& (HAS_HOLYSPEECH(speaker) || HAS_HOLYSPEECH(listener))
			)
		||  
			( // no scramble flag for language
				IS_SET(language->flags,LANGFLAG_NO_SCRAMBLE)
			)
		||  
			( // ooc rooms default to no language scrambling
				IS_OOC(listener) && !IS_SET(language->flags,LANGFLAG_SCRAMBLE_IN_OOC)
			)			
		|| 
			(
				// cases like native and alwaysunderstood
				unconditionally_understood
			)

	  )
	{
		strcpy(buf, message);
	}else{ // do some language scrambling
		buf[0]='\0';
		char dcb[2]={'\0','\0'}; // direct copy buffer
		int length;
		char *payload;
		bool force_upper;
		languagetree_data *tail;
		for(const char *pstr=message; *pstr; pstr+=length){
			// find a matching word in the language table
			length=language->find(pstr, &tail);
			if(length){
				payload=tail->stored_word;
				force_upper=length>1?false:IS_UPPER(*pstr);

				// decide if the listener understood
				if( number_percent()>ss || number_percent()>ls ){
					// not understood, copy translated
					strcat(buf, force_upper?uppercase(payload): payload);
				}else{
					strncat(buf, force_upper?uppercase(pstr): pstr, length);
				}
			}else{
				// handle non matches
				length=1;
				// copy the character not in the tables straight across
				dcb[0]=*pstr;				
				strcat( buf, dcb);

				// if colour code '`' then copy the following character also
				if(*pstr=='`'){
					dcb[0]=*(pstr+1);
					strcat( buf, dcb);
					length++;					
					
					// check for custom colour code
					if(*(pstr+1)=='='){
						dcb[0]=*(pstr+2);
						strcat( buf, dcb);
						length++;					
					}
				}				
			}

		}
	}

	// apply the reverse - even in ooc
	if(IS_SET(language->flags,LANGFLAG_REVERSE_TEXT) 
		&&	!unconditionally_understood
		&&	(number_percent()>ss || number_percent()>ls)){
		strreverse(buf);
	}

	// if they recognise the language - not in ooc rooms
	if( display_language 
		&& !IS_SET(language->flags,LANGFLAG_NO_LANGUAGE_NAME) 		
		&& !IS_OOC(listener) 
		&& get_skill(listener,language->gsn) > 11)
	{
		sprintf(buf2,"`#`M(%s)`&%s", language->name, buf);
	}else{
		strcpy(buf2,buf);
	}
	
	// load up output buffer 
	strcpy(output,buf2);
	
	// do improves on language - player to player only
	if( number_range(1,50) < str_len(buf2) 
		&& !IS_NPC(speaker) && !IS_NPC(listener) && IS_IC(speaker))
	{
		check_improve(speaker,language->gsn,true,4);
		check_improve(listener,language->gsn,true,4);
	}
}

/**************************************************************************/
void do_language(char_data *ch, char *argument)
{
    language_data *language;
	char arg[MIL];
	one_argument( argument, arg );

	if( IS_NULLSTR( arg )) {
		ch->printlnf("You're currently speaking in %s.", ch->language->name);

		// display known languages
		ch->println("You can currently speak the following languages:");
		char buf[MIL];
		for(language_data *lang=languages; lang; lang=lang->next){
			if(lang->gsn<0){
				// only show languages with a gsn
				continue;
			}
			if(IS_SET(lang->flags, LANGFLAG_IMMONLY) && !IS_IMMORTAL(ch) ){
				// don't show non imms, immortal only languages
				continue;
			}
			if(ch->language==lang){
				sprintf(buf, "`Y-->%-13s `x", capitalize(lang->name));
			}else{
				sprintf(buf, "`x   %-13s ", capitalize(lang->name));
			}

			if ( IS_SET(lang->flags, LANGFLAG_NO_SKILL_REQUIRED)){
				strcat(buf, " n/a (no skill required)");
			}else{
				// by this stage, the language must have an associated skill to be displayed
				if(lang->gsn<0){
					continue;
				}
				int sk=get_skill(TRUE_CH(ch), lang->gsn);
				if(sk<1){
					continue;
				}
				strcat(buf, FORMATF("%3d%%",sk>100?100:sk));
			}
			if(IS_SET(lang->flags, LANGFLAG_IMMONLY)){
				strcat(buf, " (immortal only)");
			}
			ch->println(buf);
		}

		return;
	}

	// look up the language
	language = language_lookup(arg);
    if( !language 
		|| (IS_SET(language->flags, LANGFLAG_IMMONLY) && !IS_IMMORTAL(ch)) ){
        ch->printlnf("No such tongue '%s' exists.", arg);
        return;
    }
   
	// spirit walking players can't use unlimited languages
    if (!IS_NPC(ch) || ( IS_CONTROLLED(ch) && get_trust(ch) < LEVEL_IMMORTAL ))
    {
		if(IS_SET(language->flags, LANGFLAG_SYSTEM_LANGUAGE)){
			if(!IS_IMMORTAL(ch)){
				ch->printlnf("No such tongue '%s' exists.", arg);   
				return;
			}
		}else{
			if ( !IS_SET(language->flags, LANGFLAG_NO_SKILL_REQUIRED) 
				&& language->gsn>=0 
				&& get_skill(TRUE_CH(ch), language->gsn)<1)
			{				
				ch->printlnf("You have no knowledge of any %s words.", language->name);
				return;
			}
		}
    }

	if( IS_SET( ch->dyn, DYN_IS_BEING_ORDERED)
		&& IS_SET(language->flags, LANGFLAG_NO_ORDER))
	{
		ch->printlnf("You can't be ordered to speak in '%s'.", language->name);   
        return;		
	}

	ch->language=language;
	ch->printlnf("Ok, you will now speak in '%s' by default.", language->name);
}

/**************************************************************************/
// return true if language matched
bool language_dynamic_command(char_data *ch, char *command, char *argument)
{
	language_data *language;
	if(IS_NULLSTR(command) || (command[0]=='-' && command[1]=='\0')){
        return false;
    }

	// try an exact match of command first
	for(language=languages; language; language=language->next){
		if(IS_SET(language->flags, LANGFLAG_IMMONLY) && !IS_IMMORTAL(ch) ){
			continue;
		}
		if(IS_SET(language->flags, LANGFLAG_NO_COMMAND_ACCESS)){
			continue;
		}
		if(!IS_NULLSTR(language->commandname) && !str_cmp(command,language->commandname)){
			break;
		}
	}
	if(!language){
		// try a prefix match if the exact match failed
		for(language=languages; language; language=language->next){
			if(IS_SET(language->flags, LANGFLAG_IMMONLY) && !IS_IMMORTAL(ch) ){
				continue;
			}
			if(IS_SET(language->flags, LANGFLAG_NO_COMMAND_ACCESS)){
				continue;
			}
			if(!IS_NULLSTR(language->commandname) && !str_prefix(command,language->commandname)){
				break;
			}
		}
	}

	if(!language){
		// the command isn't a language
		return false;
	}

	saymote( language, ch, argument, 0);
	return true;
}
/**************************************************************************/
/**************************************************************************/

