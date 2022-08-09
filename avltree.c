/* avltree.h
 *
 * Copyright 2022 Zhengyi Fu <tsingyat@outlook.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "avltree.h"
#include <assert.h>

static bool avl_is_left_child_of_parent(const struct avl_node *x)
{
    return x->avl_parent->avl_left == x;
}

static void avl_rotate_right(struct avl_node *x)
{
    /*
            x           y
           / \         / \
          y   z   ->  a   x
         / \             / \
        a   b           b   z
    */

    struct avl_node *y;

    assert(x != NULL && x->avl_left != NULL);
    y = x->avl_left;
    y->avl_parent = x->avl_parent;
    if (avl_is_left_child_of_parent(x))
        x->avl_parent->avl_left = y;
    else
        x->avl_parent->avl_right = y;
    x->avl_parent = y;
    x->avl_left = y->avl_right;
    if (y->avl_right)
        y->avl_right->avl_parent = x;
    y->avl_right = x;
}

static void avl_rotate_left(struct avl_node *x)
{
    /*
        x             y
       / \           / \
      z   y    ->   x   b
         / \       / \
        a   b     z   a
    */

    struct avl_node *y;

    assert(x != NULL && x->avl_right != NULL);
    y = x->avl_right;
    y->avl_parent = x->avl_parent;
    if (avl_is_left_child_of_parent(x))
        x->avl_parent->avl_left = y;
    else
        x->avl_parent->avl_right = y;
    x->avl_parent = y;
    x->avl_right = y->avl_left;
    if (y->avl_left)
        y->avl_left->avl_parent = x;
    y->avl_left = x;
}

struct avl_node *avl_first(const struct avl_root *tree)
{
    return tree->avl_node ? avl_min(tree->avl_node) : (struct avl_node *)tree;
}

struct avl_node *avl_last(const struct avl_root *tree)
{
    return avl_max(tree->avl_node);
}

struct avl_node *avl_min(const struct avl_node *x)
{
    struct avl_node *y = NULL;
    while (x != NULL) {
        y = (struct avl_node *)x;
        x = x->avl_left;
    }
    return y;
}

struct avl_node *avl_max(const struct avl_node *x)
{
    struct avl_node *y = NULL;
    while (x != NULL) {
        y = (struct avl_node *)x;
        x = x->avl_right;
    }
    return y;
}

/* Precondition: x != avl_first(root) */
struct avl_node *avl_prev(const struct avl_node *x)
{
    if (x->avl_left != NULL)
        return avl_max(x->avl_left);

    while (avl_is_left_child_of_parent(x))
        x = x->avl_parent;
    return x->avl_parent;
}

struct avl_node *avl_next(const struct avl_node *x)
{
    if (x->avl_right != NULL)
        return avl_min(x->avl_right);

    while (!avl_is_left_child_of_parent(x))
        x = x->avl_parent;
    return x->avl_parent;
}

/* Link node x to parent. */
void avl_link_node(struct avl_node *x, struct avl_node *parent,
                   struct avl_node **link)
{
    assert(&parent->avl_left == link || &parent->avl_right == link);
    *link = x;
    x->avl_parent = parent;
    x->avl_left = x->avl_right = NULL;
    x->avl_balance = 0;
}

void avl_balance_insert(struct avl_node *node, struct avl_root *tree)
{
    assert(node->avl_balance == 0);

    for (;;) {
        struct avl_node *parent = node->avl_parent;

        if (node == tree->avl_node)
            break;

        if (parent->avl_right == node) {
            ++parent->avl_balance;

            if (parent->avl_balance == 0)
                break;

            if (parent->avl_balance == +1) {
                node = parent;
                continue;
            }

            if (parent->avl_balance == +2) {
                if (node->avl_balance == +1) {
                    avl_rotate_left(parent);
                    parent->avl_balance = 0;
                    node->avl_balance = 0;
                } else {
                    struct avl_node *tmp = node->avl_left;
                    assert(node->avl_balance == -1);
                    avl_rotate_right(node);
                    avl_rotate_left(parent);

                    if (tmp->avl_balance == 0) {
                        node->avl_balance = parent->avl_balance = 0;
                    } else if (tmp->avl_balance == -1) {
                        node->avl_balance = +1;
                        parent->avl_balance = 0;
                    } else {
                        assert(tmp->avl_balance == +1);
                        node->avl_balance = 0;
                        parent->avl_balance = -1;
                    }
                    tmp->avl_balance = 0;
                }
                break;
            }
        } else {
            --parent->avl_balance;

            if (parent->avl_balance == 0)
                break;

            if (parent->avl_balance == -1) {
                node = parent;
                continue;
            }

            if (parent->avl_balance == -2) {
                if (node->avl_balance == -1) {
                    /*
                           p            n
                          /            / \
                         n       ->   a   p
                        / \              /
                       a   b            b
                     */
                    avl_rotate_right(parent);
                    parent->avl_balance = 0;
                    node->avl_balance = 0;
                } else {
                    /*
                               p             p         t
                              / \           / \       / \
                             n       ->    t     ->  n   p
                            / \           / \       / \ / \
                               t         n
                              / \       / \
                     */
                    struct avl_node *tmp = node->avl_right;
                    assert(node->avl_balance == +1);
                    avl_rotate_left(node);
                    avl_rotate_right(parent);

                    if (tmp->avl_balance == 0) {
                        node->avl_balance = parent->avl_balance = 0;
                    } else if (tmp->avl_balance == +1) {
                        node->avl_balance = -1;
                        parent->avl_balance = 0;
                    } else {
                        assert(tmp->avl_balance == -1);
                        node->avl_balance = 0;
                        parent->avl_balance = +1;
                    }
                    tmp->avl_balance = 0;
                }
                break;
            }
        }
    }
}
