/* rbtree.c
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

#include "rbtree.h"
#include <assert.h>

/*
  The root node's rb_parent will be the address of struct rb_root.
  When accessing struct rb_root through (struct rb_node *), only
  the rb_left member is valid, which corresponds to rb_node.
*/

/* Returns true if x is left child of its parent or x is the root node. */
static bool rb_is_left_child_of_parent(const struct rb_node *x)
{
    return (x->rb_parent->rb_left == x);
}

static void rb_rotate_right(struct rb_node *x)
{
    /*
            x           y
           / \         / \
          y   z   ->  a   x
         / \             / \
        a   b           b   z
    */

    struct rb_node *y;

    assert(x != NULL && x->rb_left != NULL);
    y = x->rb_left;
    y->rb_parent = x->rb_parent;
    if (rb_is_left_child_of_parent(x))
        x->rb_parent->rb_left = y;
    else
        x->rb_parent->rb_right = y;
    x->rb_parent = y;
    x->rb_left = y->rb_right;
    if (y->rb_right)
        y->rb_right->rb_parent = x;
    y->rb_right = x;
}

static void rb_rotate_left(struct rb_node *x)
{
    /*
        x             y
       / \           / \
      z   y    ->   x   b
         / \       / \
        a   b     z   a
    */

    struct rb_node *y;

    assert(x != NULL && x->rb_right != NULL);
    y = x->rb_right;
    y->rb_parent = x->rb_parent;
    if (rb_is_left_child_of_parent(x))
        x->rb_parent->rb_left = y;
    else
        x->rb_parent->rb_right = y;
    x->rb_parent = y;
    x->rb_right = y->rb_left;
    if (y->rb_left)
        y->rb_left->rb_parent = x;
    y->rb_left = x;
}

struct rb_node *rb_first(const struct rb_root *tree)
{
    return tree->rb_node ? rb_min(tree->rb_node) : (struct rb_node *)tree;
}

struct rb_node *rb_last(const struct rb_root *tree)
{
    return rb_max(tree->rb_node);
}

struct rb_node *rb_min(const struct rb_node *x)
{
    struct rb_node *y = NULL;
    while (x != NULL) {
        y = (struct rb_node *)x;
        x = x->rb_left;
    }
    return y;
}

struct rb_node *rb_max(const struct rb_node *x)
{
    struct rb_node *y = NULL;
    while (x != NULL) {
        y = (struct rb_node *)x;
        x = x->rb_right;
    }
    return y;
}

/* Precondition: x != rb_first(root) */
struct rb_node *rb_prev(const struct rb_node *x)
{
    if (x->rb_left != NULL)
        return rb_max(x->rb_left);

    while (rb_is_left_child_of_parent(x))
        x = x->rb_parent;
    return x->rb_parent;
}

struct rb_node *rb_next(const struct rb_node *x)
{
    if (x->rb_right != NULL)
        return rb_min(x->rb_right);

    while (!rb_is_left_child_of_parent(x))
        x = x->rb_parent;
    return x->rb_parent;
}

/* Link node x to parent. */
void rb_link_node(struct rb_node *x, struct rb_node *parent,
                  struct rb_node **link)
{
    assert(&parent->rb_left == link || &parent->rb_right == link);
    *link = x;
    x->rb_parent = parent;
    x->rb_left = x->rb_right = NULL;
    x->rb_is_black = false;
}

/* Rebalance after inserting node x into tree root. */
void rb_balance_insert(struct rb_node *x, struct rb_root *root)
{
    x->rb_is_black = x == root->rb_node;

    while (x != root->rb_node && !x->rb_parent->rb_is_black) {
        if (rb_is_left_child_of_parent(x->rb_parent)) {
            struct rb_node *y = x->rb_parent->rb_parent->rb_right;

            if (y != NULL && !y->rb_is_black) {
                /*
                  Case 1: p is red & y is red

                            g
                           / \
                          p   y
                         / \
                        x   x

                        We violate the rule that a red node cannot have red
                   children. So we color p and y black, and g red. This doesn't
                   change black height, but g's parent may still be red. So we
                   continue to rebalance at g.
                 */
                x = x->rb_parent;
                x->rb_is_black = true;
                y->rb_is_black = true;
                x = x->rb_parent;
                x->rb_is_black = x == root->rb_node;
            } else { /* y == NULL || y->is_black */
                if (!rb_is_left_child_of_parent(x)) {
                    /*
                      Case 2: y is black & x is p's right child.
                      x and p are red.

                             g           g
                            / \         / \
                           p   y  ->   x   y
                            \         /
                            x         p

                      Left rotate at p to transform into case 3.
                    */
                    x = x->rb_parent;
                    rb_rotate_left(x);
                } /* !rb_is_left_child_of_parent(x) */

                /*
                  Case 3: y is black & x is p's left child.
                  x and p are red.

                         g               p
                        / \             / \
                       p   y     ->    x   g
                      /                     \
                     x                       y

                  We color g red and p black,
                  And rotate right at g.
                  As a result we transfer one of the two red nodes
                  of the left subtree to the right subtree.
                */

                x = x->rb_parent;
                x->rb_is_black = true;
                x = x->rb_parent;
                x->rb_is_black = false;
                rb_rotate_right(x);
                break;
            }
        } else { /* !rb_is_left_child_of_parent(x->rb_parent) */
            struct rb_node *y = x->rb_parent->rb_parent->rb_left;

            if (y != NULL && !y->rb_is_black) {
                x = x->rb_parent;
                x->rb_is_black = true;
                y->rb_is_black = true;
                x = x->rb_parent;
                x->rb_is_black = x == root->rb_node;
            } else { /* y == NULL || y->rb_is_black */
                if (rb_is_left_child_of_parent(x)) {
                    x = x->rb_parent;
                    rb_rotate_right(x);
                }
                x = x->rb_parent;
                x->rb_is_black = true;
                x = x->rb_parent;
                x->rb_is_black = false;
                rb_rotate_left(x);
                break;
            }
        }
    } /* while (x != root && !x->rb_parent->rb_is_black) */
}

void rb_remove(struct rb_node *x, struct rb_root *root)
{
    /*
      y is either x or x's successor,
      which will have at most one child.
    */
    struct rb_node *y = x;

    /* z will be y's (possibly null) single child. */
    struct rb_node *z = NULL;

    /*	w is z's uncle, and will be z's sibling. */
    struct rb_node *w = NULL;

    /* y's original color. */
    bool remove_black;

    /* find y */
    if (x->rb_left && x->rb_right) {
        y = x->rb_right;
        while (y->rb_left)
            y = y->rb_left;
    }

    z = y->rb_left ? y->rb_left : y->rb_right;

    /* find w */
    if (y != root->rb_node) {
        if (rb_is_left_child_of_parent(y))
            w = y->rb_parent->rb_right;
        else
            w = y->rb_parent->rb_left;
    }

    remove_black = y->rb_is_black;

    /* remove y */
    if (z != NULL)
        z->rb_parent = y->rb_parent;
    if (rb_is_left_child_of_parent(y))
        y->rb_parent->rb_left = z;
    else
        y->rb_parent->rb_right = z;

    if (x != y) {
        /* replace x by y */
        y->rb_left = x->rb_left;
        if (x->rb_left)
            x->rb_left->rb_parent = y;
        y->rb_right = x->rb_right;
        if (x->rb_right)
            x->rb_right->rb_parent = y;
        y->rb_parent = x->rb_parent;
        if (rb_is_left_child_of_parent(x))
            x->rb_parent->rb_left = y;
        else
            x->rb_parent->rb_right = y;
        y->rb_is_black = x->rb_is_black;
    }

    if (remove_black) {
        /* rebalance if we removed a black node */

        for (;;) {
            if (root->rb_node == NULL)
                break;

            if (z == root->rb_node) {
                z->rb_is_black = true;
                break;
            }

            if (z != NULL && !z->rb_is_black) {
                /*
                   Case 1: z is red.
                   Color z black.
                   Done
                */

                z->rb_is_black = true;
                break;
            }

            if (w != NULL && !w->rb_is_black) {
                /*
                   Case 2: z is black & w is red.

                           p            w
                          / \          / \
                        *z   w   ->   p  d
                            / \      / \
                            c d      z c

                   Left rotate at p to turn into other cases.
                */
                if (rb_is_left_child_of_parent(w))
                    rb_rotate_right((w = w->rb_parent));
                else
                    rb_rotate_left((w = w->rb_parent));
                w->rb_is_black = false;
                w->rb_parent->rb_is_black = true;
                w = w->rb_left == z ? w->rb_right : w->rb_left;
                assert(w != NULL);
                assert(w->rb_is_black);
            }

            if ((w->rb_left == NULL || w->rb_left->rb_is_black) &&
                (w->rb_right == NULL || w->rb_right->rb_is_black)) {
                /*
                   Case 3: z and w are black, w has no red children

                         p
                        / \
                      *z   w
                          / \
                          b c

                   We decrease the black height of the right subtree by coloring
                   w red. If p is red, painting p black will increase black
                   height of both subtree. Otherwise, assign p to z and continue
                   to rebalance.
                */
                w->rb_is_black = false;
                z = w->rb_parent;
                if (z == root->rb_node)
                    break;
                if (rb_is_left_child_of_parent(z))
                    w = z->rb_parent->rb_right;
                else
                    w = z->rb_parent->rb_left;
                assert(w != NULL);
            } else {
                if (!rb_is_left_child_of_parent(w)) {
                    if (w->rb_left != NULL && !w->rb_left->rb_is_black) {
                        /*
                           Case 4:
                                p              p
                               / \            / \
                              z   w    ->    z   c
                                 /                \
                                 c                 w
                         */
                        w->rb_is_black = false;
                        w->rb_left->rb_is_black = true;
                        rb_rotate_right(w);
                        w = w->rb_parent;
                    }

                    /*
                       Case 5:
                           p                w
                          / \              / \
                         z   w       ->   p   d
                            / \          / \
                            c d          z c
                     */
                    w->rb_is_black = w->rb_parent->rb_is_black;
                    w->rb_right->rb_is_black = true;
                    w = w->rb_parent;
                    assert(w != NULL);
                    w->rb_is_black = true;
                    rb_rotate_left(w);
                    break;
                } else {
                    /* Symmetric cases 4 & 5. */
                    if (w->rb_right != NULL && !w->rb_right->rb_is_black) {
                        w->rb_is_black = false;
                        w->rb_right->rb_is_black = true;
                        rb_rotate_left(w);
                        w = w->rb_parent;
                    }

                    w->rb_is_black = w->rb_parent->rb_is_black;
                    w->rb_left->rb_is_black = true;
                    w = w->rb_parent;
                    assert(w != NULL);
                    w->rb_is_black = true;
                    rb_rotate_right(w);
                    break;
                }
            }
        }
    }
}
