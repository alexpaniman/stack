#include "akinator.h"
#include "graphviz.h"
#include <cstdio>

int main(void) {
    const char* akinator_database_file_name =
        "/home/alex/projects/stack/akinator/akinator.tree";

    binary_tree<akinator_node>* tree =
        akinator_read_tree(akinator_database_file_name);

    digraph graph = akinator_visualize(tree);
    digraph_render_and_destory(&graph);

    game_mode mode = read_game_mode();

    switch(mode) {
    case MODE_PLAY:
        akinator_play(tree);
        break;

    case MODE_DEFINE:
        akinator_ask_and_define(tree);
        break;

    case MODE_COMPARE:
        akinator_ask_and_find_difference(tree);
        break;
    }

    akinator_write_tree(akinator_database_file_name, tree);
    akinator_destroy(tree);
}
