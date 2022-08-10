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


void avl_erase(struct avl_node *x, struct avl_root *tree)
{
    /*
      y is either x or x's successor,
      which will have at most one child.
    */
    struct avl_node *y = x;

    /* z will be y's (possibly null) single child. */
    struct avl_node *z = NULL;

    /*  p is y's parent or y */
    struct avl_node *p = NULL;


    /* find y */
    if (x->avl_left && x->avl_right) {
        y = x->avl_right;
        while (y->avl_left)
            y = y->avl_left;
    }

    z = y->avl_left ? y->avl_left : y->avl_right;

    if (y != tree->avl_node) {
        p = y->avl_parent;

        if (p->avl_left == y)
            p->avl_balance += 1;
        else
            p->avl_balance -= 1;
    }

    if (p == x)
        p = y;

    /* remove y */
    if (z != NULL)
        z->avl_parent = y->avl_parent;
    if (avl_is_left_child_of_parent(y))
        y->avl_parent->avl_left = z;
    else
        y->avl_parent->avl_right = z;

    if (x != y) {
        /* replace x by y */
        y->avl_left = x->avl_left;
        if (x->avl_left)
            x->avl_left->avl_parent = y;
        y->avl_right = x->avl_right;
        if (x->avl_right)
            x->avl_right->avl_parent = y;
        y->avl_parent = x->avl_parent;
        if (avl_is_left_child_of_parent(x))
            x->avl_parent->avl_left = y;
        else
            x->avl_parent->avl_right = y;
        y->avl_balance = x->avl_balance;
    }

    for (;;) {
        if (p == NULL || p == tree->avl_node)
            break;

        if (p->avl_balance == 0) {
            if (avl_is_left_child_of_parent(p))
                p->avl_parent->avl_balance++;
            else
                p->avl_parent->avl_balance--;
            p = p->avl_parent;
        } else if (p->avl_balance == +1 || p->avl_balance == -1) {
            break;
        } else if (p->avl_balance == +2) {
            /*
                   p
                  / \
                 c   w
                    / \
                    a b
             */
            struct avl_node *w = p->avl_right;
            if (w->avl_balance == 0) {
                /*
                  h(a) = h(c) = h > 0
                  h(c) = h - 1

                       w
                      / \
                     p   b
                    / \
                    c a
                 */
                avl_rotate_left(p);
                w->avl_balance = -1;
                p->avl_balance = +1;
                break;
            } else if (w->avl_balance == +1) {
                /*
                   h(b) = h > 0
                   h(a) = h(c) = h - 1
                 */
                avl_rotate_left(p);
                w->avl_balance = 0;
                p->avl_balance = 0;
                p = w->avl_parent;
                if (p->avl_left == w)
                    p->avl_balance += 1;
                else
                    p->avl_balance -= 1;
            } else {
                assert(w->avl_balance == -1);
                /*
                   h(a) = h > 0
                   h(b) = h(c) = h - 1

                       p            p               a
                      / \          / \             / \
                     c   w        c   a           p   w
                        / \   ->     / \     ->  / \ / \
                       a   b        u   w       c  u v  b
                      / \              / \
                     u   v            v   b
                 */
                struct avl_node *a = w->avl_left;
                assert(a != NULL);

                avl_rotate_right(w);
                avl_rotate_left(p);

                if (a->avl_balance == 0) {
                    /* h(u) = h(v) = h(b) = h(c) = h - 1 */
                    p->avl_balance = w->avl_balance = 0;
                } else if (a->avl_balance == +1) {
                    /* h(u) = h - 2, h(v) = h - 1 */
                    w->avl_balance = 0;
                    p->avl_balance = -1;
                } else {
                    assert (a->avl_balance == -1);
                    /* h(u) = h - 1, h(v) = h - 2 */
                    p->avl_balance = 0;
                    w->avl_balance = +1;
                }

                a->avl_balance = 0;
                p = a->avl_parent;
                if (p->avl_left == a)
                    p->avl_balance += 1;
                else
                    p->avl_balance -= 1;
            }
        } else {
            struct avl_node *w = p->avl_left;
            assert(p->avl_balance == -2);

            if (w->avl_balance == 0) {
                avl_rotate_right(p);
                w->avl_balance = +1;
                p->avl_balance = -1;
                break;    
            } else if (w->avl_balance == -1) {
                avl_rotate_right(p);
                w->avl_balance = 0;
                p->avl_balance = 0;
                p = w->avl_parent;
                if (p->avl_left == w)
                    p->avl_balance += 1;
                else
                    p->avl_balance -= 1;
            } else {
                assert(w->avl_balance == +1);

                struct avl_node *a = w->avl_right;
                assert(a != NULL);

                avl_rotate_left(w);
                avl_rotate_right(p);

                if (a->avl_balance == 0) {
                    p->avl_balance = w->avl_balance = 0;
                } else if (a->avl_balance == -1) {
                    w->avl_balance = 0;
                    p->avl_balance = +1;
                } else {
                    assert (a->avl_balance == +1);
                    p->avl_balance = 0;
                    w->avl_balance = -1;
                }

                a->avl_balance = 0;
                p = a->avl_parent;
                if (p->avl_left == a)
                    p->avl_balance += 1;
                else
                    p->avl_balance -= 1;
            }
        }
    }
}
