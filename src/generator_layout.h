#ifndef _GENERATOR_LAYOUT_H_
#define _GENERATOR_LAYOUT_H_

#include <stddef.h>

#include "base.h"
#include "memory.h"
#include "string.h"
#include "generator.h"
#include "ast.h"
#include "ast_layout.h"

/**
 *
 * @function generator_code_layout_block
 * @brief Generate the HTML code for the layout block
 * @params {generator_t*} generator - Generator
 * @params {array_t*} children - Children
 * @returns {string_t*}
 *
 */
string_t* generator_code_layout_block(generator_t* generator, array_t* children);

/**
 * 
 * @function generator_code_layout_body
 * @params {generator_t*} generator - Generator
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} body - Body
 * @returns {void}
 * 
 */
void generator_code_layout_body(generator_t* generator, ast_layout_block_t* layout_block, string_t* body);


/**
 * 
 * @function generator_code_head_item
 * @params {ast_layout_attribute_t*} attribute - Attribute
 * @params {string_t*} head - Head
 * @returns {void}
 * 
 */
void generator_code_head_item(ast_layout_attribute_t* attribute, string_t* head);

/**
 * 
 * @function generator_code_head
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} head - Head
 * @returns {void}
 * 
 */
void generator_code_head(ast_layout_block_t* layout_block, string_t* head);

/**
 *
 * @function generator_code_layout
 * @brief Generate the HTML code for the layout block
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_code_layout(generator_t* generator);

/**
 * 
 * @function generator_code_layout_style
 * @brief Generate the CSS code for the layout block
 * @params {hashmap_attribute_t*} styles - Styles
 * @params {ast_layout_block_t*} block - Layout block
 * @params {size_t*} css_attributes_length - CSS attributes length
 * @returns {string_t*}
 * 
 */
string_t* generator_code_layout_style(hashmap_attribute_t* styles, ast_layout_block_t* block, size_t* css_attributes_length);


/**
 *
 * @function generator_code_layout_html
 * @brief Generate the HTML code for the layout block
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} html - HTML string
 * @returns {void}
 *
 */
void generator_code_layout_html(ast_layout_block_t* layout_block, string_t* html);

/**
 *
 * @function generator_code_layout_attributes
 * @brief Generate the HTML code for the layout block attributes
 * @params {generator_t} generator - Generator
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {string_t*}
 *
 */
string_t* generator_code_layout_attributes(generator_t* generator, ast_layout_block_t* block);

/**
 * 
 * @function generator_code_layout_style_value
 * @brief Convert AST layout attribute values to CSS attribute values
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New Styles
 * @params {ast_layout_attribute_t*} attribute - Layout Attribute
 * @params {ast_layout_node_type_t} parent_node_type - Parent Node Type
 * @returns {void}
 * 
 */
void generator_code_layout_style_value(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, ast_layout_node_type_t parent_node_type);

#endif
