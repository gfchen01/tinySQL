/**
 * @file parser.h
 * @author CHEN Guofei (simonsatzju@gmail.com)
 * @brief A SQL parser, thanks to an open source project [SQL-PARSER]
 * @version 0.1
 * @date 2022-05-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __PARSER_H__
#define __PARSER_H__

// #include 

/**
 * @brief The execute type of syntax tree.
 * 
 */
enum syntax_node_type{
    NodeQuit, ///< Quit application
    NodeInsert, ///< Insert table
    //... TODO: Other types
};
/**
 * @brief 
 * 
 */
struct syntax_tree_node{
    
};

/**
 * @brief 
 * 
 * @param arg1 
 * @param arg2 
 */
void foo(int arg1, int arg2);

#endif