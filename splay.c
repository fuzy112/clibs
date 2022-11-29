/* splay.c
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

#include "splay.h"
#include <assert.h>

static bool splay_is_left_child_of_parent(const struct splay_node *n)
{
    return n->splay_parent->splay_left == n;
}

struct splay_node *splay_first(const struct splay_root *tree)
{
    return tree->splay_node ? splay_min(tree->splay_node)
                            : (struct splay_node *)tree;
}

struct splay_node *splay_last(const struct splay_root *tree)
{
    return splay_max(tree->splay_node);
}

struct splay_node *splay_min(const struct splay_node *x)
{
    struct splay_node *y = NULL;
    while (x != NULL) {
        y = (struct splay_node *)x;
        x = x->splay_left;
    }
    return y;
}

struct splay_node *splay_max(const struct splay_node *x)
{
    struct splay_node *y = NULL;
    while (x != NULL) {
        y = (struct splay_node *)x;
        x = x->splay_right;
    }
    return y;
}

/* Precondition: x != splay_first(root) */
struct splay_node *splay_prev(const struct splay_node *x)
{
    if (x->splay_left != NULL)
        return splay_max(x->splay_left);

    while (splay_is_left_child_of_parent(x))
        x = x->splay_parent;
    return x->splay_parent;
}

struct splay_node *splay_next(const struct splay_node *x)
{
    if (x->splay_right != NULL)
        return splay_min(x->splay_right);

    while (!splay_is_left_child_of_parent(x))
        x = x->splay_parent;
    return x->splay_parent;
}

/* Link node x to parent. */
void splay_link_node(struct splay_node *x, struct splay_node *parent,
                     struct splay_node **link)
{
    assert(&parent->splay_left == link || &parent->splay_right == link);
    *link = x;
    x->splay_parent = parent;
    x->splay_left = x->splay_right = NULL;
}

void splay_balance(struct splay_node *node, struct splay_root *tree)
{
    for (;;) {
        struct splay_node *const parent = node->splay_parent;

        if (node == tree->splay_node)
            break;

        if (parent == tree->splay_node) {
            if (parent->splay_left == node) {
                node->splay_parent = (struct splay_node *)tree;
                tree->splay_node = node;
                parent->splay_left = node->splay_right;
                if (node->splay_right) {
                    node->splay_right->splay_parent = parent;
                }
                node->splay_right = parent;
                parent->splay_parent = node;
            } else {
                node->splay_parent = (struct splay_node *)tree;
                tree->splay_node = node;
                parent->splay_right = node->splay_left;
                if (node->splay_left) {
                    node->splay_left->splay_parent = parent;
                }
                node->splay_left = parent;
                parent->splay_parent = node;
            }
            break;
        }

        if (parent->splay_left == node) {
            struct splay_node *gparent = parent->splay_parent;
            if (gparent->splay_left == parent) {
                node->splay_parent = gparent->splay_parent;
                if (splay_is_left_child_of_parent(gparent))
                    gparent->splay_parent->splay_left = node;
                else
                    gparent->splay_parent->splay_right = node;
                parent->splay_parent = node;
                parent->splay_left = node->splay_right;
                if (node->splay_right)
                    node->splay_right->splay_parent = parent;
                node->splay_right = parent;
                gparent->splay_parent = parent;
                gparent->splay_left = parent->splay_right;
                if (parent->splay_right)
                    parent->splay_right->splay_parent = gparent;
                parent->splay_right = gparent;
            } else {
                node->splay_parent = gparent->splay_parent;
                if (!splay_is_left_child_of_parent(gparent))
                    gparent->splay_parent->splay_right = node;
                else
                    gparent->splay_parent->splay_left = node;
                gparent->splay_parent = node;

                gparent->splay_right = node->splay_left;
                if (node->splay_left)
                    node->splay_left->splay_parent = gparent;
                node->splay_left = gparent;

                parent->splay_parent = node;
                parent->splay_left = node->splay_right;
                if (node->splay_right)
                    node->splay_right->splay_parent = parent;
                node->splay_right = parent;
            }

        } else {

            struct splay_node *gparent = parent->splay_parent;
            if (gparent->splay_right == parent) {
                node->splay_parent = gparent->splay_parent;
                if (splay_is_left_child_of_parent(gparent))
                    gparent->splay_parent->splay_left = node;
                else
                    gparent->splay_parent->splay_right = node;
                parent->splay_parent = node;
                parent->splay_right = node->splay_left;
                if (node->splay_left)
                    node->splay_left->splay_parent = parent;
                node->splay_left = parent;
                gparent->splay_parent = parent;
                gparent->splay_right = parent->splay_left;
                if (parent->splay_left)
                    parent->splay_left->splay_parent = gparent;
                parent->splay_left = gparent;
            } else {
                node->splay_parent = gparent->splay_parent;
                if (splay_is_left_child_of_parent(gparent))
                    gparent->splay_parent->splay_left = node;
                else
                    gparent->splay_parent->splay_right = node;
                gparent->splay_parent = node;

                gparent->splay_left = node->splay_right;
                if (node->splay_right)
                    node->splay_right->splay_parent = gparent;
                node->splay_right = gparent;

                parent->splay_parent = node;
                parent->splay_right = node->splay_left;
                if (node->splay_left)
                    node->splay_left->splay_parent = parent;
                node->splay_left = parent;
            }
        }
    }
}

void splay_erase(struct splay_node *x, struct splay_root *root)
{
    /*
      y is either x or x's successor,
      which will have at most one child.
    */
    struct splay_node *y = x;

    /* z will be y's (possibly null) single child. */
    struct splay_node *z = NULL;

    (void)root;

    /* find y */
    if (x->splay_left && x->splay_right) {
        y = x->splay_right;
        while (y->splay_left)
            y = y->splay_left;
    }

    z = y->splay_left ? y->splay_left : y->splay_right;

    /* remove y */
    if (z != NULL)
        z->splay_parent = y->splay_parent;
    if (splay_is_left_child_of_parent(y))
        y->splay_parent->splay_left = z;
    else
        y->splay_parent->splay_right = z;

    if (x != y) {
        /* replace x by y */
        y->splay_left = x->splay_left;
        if (x->splay_left)
            x->splay_left->splay_parent = y;
        y->splay_right = x->splay_right;
        if (x->splay_right)
            x->splay_right->splay_parent = y;
        y->splay_parent = x->splay_parent;
        if (splay_is_left_child_of_parent(x))
            x->splay_parent->splay_left = y;
        else
            x->splay_parent->splay_right = y;
    }
}
