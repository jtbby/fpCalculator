#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tinysf.h"
#include "hashmap.h"

// Symbol Table Struct Definitions
typedef struct sym_struct {
  char name[255];
  tinysf_s val;
} Sym_t;

// Function to free the Value, passed in to HashMap on create
static void freesym(void *ptr) {
  if(ptr == NULL) {
    return;
  }

  Sym_t *sym = (Sym_t *)ptr;
  free(sym);
}

// Local Support Function to Create a Symbol
static Sym_t *create_sym(const char *var, tinysf_s val) {
  Sym_t *sym = calloc(1, sizeof(Sym_t));
  if(sym == NULL) {
    printf("Error: Can't Create Symbol\n");
    exit(1);
  }
  strncpy(sym->name, var, 255);
  sym->val = val;
  return sym;
}

// Initialized the symbol table (hashmap implementation)
void initialize_symtab() {
  hashmap_create(10, freesym);
}

// Helper to search a hashmap to see if the key exists
int sym_exists(const char *name) {
  return hashmap_containsKey(name);
}

// Insert a new symbol into the hashmap
void insert_symbol(const char *name, tinysf_s value) {
  Sym_t *sym = create_sym(name, value);
  hashmap_put(sym->name, sym);
}

// Search the hashmap and return the value
tinysf_s get_value(const char *name) {
  Sym_t *sym = hashmap_get(name);
  if(sym) {
    return sym->val;
  }
  else {
    return -1;
  }
}

// Clean up the symbol table
void teardown_symtab() {
  hashmap_destroy();
}