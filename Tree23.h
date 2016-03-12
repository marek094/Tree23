/* 
 * File:   Tree23.h
 * Author: MarekCerny.com
 */

#include <cassert>
#ifdef TREE23_DEBUG
#include <iostream>
#include <queue>
#define DBG_OUT(x) std::cout << x
#else
#define DBG_OUT(x) 
#endif

/* older than c++11 */
#if __cplusplus < 201103L
#define nullptr NULL
#endif


template <class T>
class Tree23 {
private: 
    
#ifdef TREE23_EXPERIMENTAL 
#define PEG_SIZE 4096  
    template <class A>
    class Alloc {
        A *peg[PEG_SIZE];
        unsigned size;        
    public:        
        Alloc () : size(0) {}
        inline A *__new() {
            if (size > 0) {
                size--;
                return peg[size];
            }
            return new A();             
        }
        inline void __delete(A **p) {
            if (size < PEG_SIZE) {
                peg[size] = *p;
                size++;
                return;
            }
            delete(*p);        
        }
        /*static void* operator new(std::size_t sz) {
            if (inst == nullptr) inst = ::new Alloc;
            if (size > 0) {
                size--;
                return peg[size];
            } 
            return ::operator new(sz);              
        }     
        static void operator delete(void *p, std::size_t sz) {
            if (size < PEG_SIZE) {
                peg[size] = (A*)p;
                size++;
                return;
            }
            ::operator delete(p);
        }*/            
    };
#endif    
  
    class Node {
    public:

        unsigned size;
        T *key[2];
        Node *child[3], *parent;

        Node() {
            child[0] = child[1] = child[2] = parent = nullptr;
            size = 0;
        }

        Node(T *_key) {
            child[0] = child[1] = child[2] = parent = nullptr;
            size = 1;
            key[0] = _key;
        }
        
        inline void addChild(int order, Node *p) {
            if (order > size) return;
            child[order] = p;
            if (p != nullptr) {
                child[order]->parent = this;
            }
        }

        inline void pushLeft(T *_key, Node *_child) {
            size++; 
            // shift right
            key[1] = key[0];
            child[2] = child[1];
            child[1] = child[0];
            // add
            key[0] = _key;
            addChild(0, _child); 
        }

        inline void pushRight(T *_key, Node *_child) {
            size++;
            key[size - 1] = _key;
            addChild(size, _child);
        }

        inline void popLeft() {
             // shift left
            key[0] = key[1];
            child[0] = child[1];
            child[1] = child[2];
            size--;
        }

        inline void popRight() {              
            size--;
        }

        inline bool leaf() {
            return child[0] == nullptr;
        }
    };

    Node *root;
    unsigned _size;
#ifdef TREE23_EXPERIMENTAL
    Alloc<Node> mem_Node;
    Alloc<T> mem_T;
#endif
    
public:
    
    Tree23() { __constructor(); }
    ~Tree23() { __destructor(&root); }

    bool find(const T &val) {
        Node *it = root;
        while (it != nullptr) {
            if (val < *it->key[0]) {
                it = it->child[0];
            } else if (!(*it->key[0] < val)) {
                return true;
            } else if (it->size == 1 || val < *it->key[1]) {
                it = it->child[1];
            } else if (!(*it->key[1] < val)) {
                return true;
            } else {
                it = it->child[2];
            }   
        }
        return false;
    }

    void insert(const T &val) {
        // = Down phase = 
        Node *act = findSmallerPtr(val);
        
        //T *key = new T(val);
        T *key = __alloc_key(); *key = val;
        _size++;
        
        // = Terminal cases =
        if (act == nullptr) {
            // empty tree                    
            if (root == nullptr) {
                root = __alloc_node(); *root = Node(key);
            }
            return;   
        } 

        // 2 node - simple add
        if (act->size == 1) {   
            act->size++;
            act->key[1] = key;
            compare(&act->key[0], &act->key[1]);
            return;
        } 

        // 3 node
        if (act->size == 2) {  
            //         |            act(k0)
            //   act(k0,k1)  --->    /    \
            //    + Cn(val)      Cn(val) Cm(k1)

            Node *cn = __alloc_node(); *cn = Node(key);
            Node *cm = __alloc_node(); *cm = Node(act->key[1]);  

            act->addChild(0, cn);
            act->addChild(1, cm);
            act->popRight();

            // sort(act->key[0], cn->key[0], cm->key[0]);
            compare(&cn->key[0], &act->key[0]);
            compare(&act->key[0], &cm->key[0]);

            // = Upward phase, cases =
            while (act != root) {
                Node *par = act->parent;

                // add ins to par(ent)
                if (par->size == 1) {                
                    if (*act->key[0] < *par->key[0]) { 
                        DBG_OUT("I2->\n");                        
                        //    par(k0)       par(l0,k0)
                        //    /   \   --->    /  |  \
                        // act(l0)  C1        D0  D1 C1
                        //  /  \
                        // D0  D1

                        par->pushLeft(act->key[0], act->child[0]);
                        par->addChild(1, act->child[1]);
                        __free_node(&act);
                        
                    } else {
                        DBG_OUT("I2<-\n");                                                
                        //  par(k0)         par(k0,l0)
                        //  / \      --->    /  |  \
                        // C0 act(l0)       C0  D0 D1
                        //     / \
                        //    D0 D1

                        par->pushRight(act->key[0], act->child[1]);
                        par->addChild(1, act->child[0]);  
                        __free_node(&act);
                        
                    }
                    // the end
                    return;
                } 

                if (par->size == 2) {

                    if (*act->key[0] < *par->key[0]) {
                        DBG_OUT("I3a\n");                                                                        
                        //          |                    |
                        //      par(k0,k1)            par(k0)
                        //      /   |   \    --->     /     \
                        //    act   D1   D2         act    Cn(k1)  
                        //    / \                   / \     / \
                        //   C0 C1                 C0 C1   D1 D2 

                        Node *cn = __alloc_node(); *cn = Node();                     
                        cn->addChild(0, par->child[1]);
                        cn->pushRight(par->key[1], par->child[2]);

                        par->addChild(1, cn);
                        par->popRight();
                        
                    } else if (*act->key[0] < *par->key[1]) {
                        DBG_OUT("I3b\n");                                                                                                
                        //          |                    |
                        //      par(k0,k1)            par(l0)
                        //      /   |   \    --->     /     \
                        //    D0 act(l0) D2       act(k0)  Cn(k1)  
                        //         / \              / \     / \
                        //        C0 C1            D0 C0   C1 D2 

                        Node *cn = __alloc_node(); *cn = Node();                     
                        cn->addChild(0, act->child[1]);
                        cn->pushRight(par->key[1], par->child[2]);
                        //(*par->key[1])++;
                        act->pushLeft(par->key[0], par->child[0]);
                        par->key[1] = act->key[1];
                        act->popRight();

                        par->addChild(2, cn);
                        par->popLeft();
                        
                    } else {
                        DBG_OUT("I3c\n");                                                                                                
                        //          |                    |
                        //      par(k0,k1)             par(k1)
                        //      /   |   \     --->    /     \
                        //     C0   C1  act        Cn(k0)   act
                        //              / \         / \     / \
                        //             D0 D1       C0 C1   D0 D1 

                        Node *cn = __alloc_node(); *cn = Node();                     
                        cn->addChild(0, par->child[0]);
                        cn->pushRight(par->key[0], par->child[1]);

                        par->addChild(1, cn);
                        par->popLeft();

                    }
                } else {
                    // ERROR
                    assert(false);
                } 

                // Continue
                act = par;

            } /* while (act != root) */

        }

        return;
    }      

    void erase(const T &val) { 
        // = Downward Phase =
        Node *it = findPtr(val);
        
        // not in tree
        if (it == nullptr) return;
        _size--;

        // which key in node?
        int key_i = 0;
        while (*it->key[key_i] != val) key_i++;

        // find neaactt leaf value
        Node *act = it;
        if ( !act->leaf() ) {
            act = it->child[key_i];
            while ( !act->leaf() ) {              
                act = act->child[act->size];
            }
        }

        // replace erased value with leaf value
        __free_key(&it->key[key_i]);
        it->key[key_i] = act->key[act->size-1];                           
        act->popRight();   
        
#ifdef TREE23_DEBUG
        dbg_bfs();
#endif          
        /* Upward Phase */
        if (act->size == 1) return;

        // its is hole
        if (act->size == 0) { 
            Node *par, **c; // parent, [C0,C1,C2]

            while (act != root) {
                par = act->parent;
                c = par->child;

                if (par->size == 1 && c[0]->size == 1) {
                    DBG_OUT("\\\n");
                    //      |                   |
                    //   par(k0)               par()
                    //    /    \                | 
                    // C0(l0) act()  --->   C0(l0,k0)
                    //  / \     |            /  |  \
                    // D0 D1    E0          D0  D1  E0

                    c[0]->pushRight(par->key[0], act->child[0]);
                    par->popRight();         
                    __free_node(&act);
                    
                } else if (par->size == 1 && c[1]->size == 1) {
                    DBG_OUT("/\n");                        
                    //      |                   |
                    //   par(k0)               par()
                    //   /    \                 | 
                    // act()  C1(l0)  --->   C1(k0,l0)
                    //  |     /  \           /  |  \
                    // D0    E0  E1         D0  E0  E1

                    c[1]->pushLeft(par->key[0], act->child[0]);
                    par->addChild(0, c[1]);
                    par->popRight();         
                    __free_node(&act);
                    
                } else {
                    // no need loop
                    break;
                }

                act = par;

#ifdef TREE23_DEBUG
                dbg_bfs();
#endif                   
            } /* while (act != root) */

            // root is hole
            if (act == root && act->size == 0) {
                root = act->child[0];
                return;
            }                           
            // Enjoying special cases 
            par = act->parent;
            c = par->child; 
            if (par->size == 1) {
                if (c[0]->size == 2) {
                    DBG_OUT("2->\n");                        
                    //       |                      |
                    //     par(k0)               par(l1)
                    //     /     \                /   \
                    // C0(l0,l1) act()  --->  C0(l0) act(k0)
                    //  / |  \    |             / \   / \
                    // D0 D1 D2   E0           D0 D1 D2 E0 

                    act->pushLeft(par->key[0], c[0]->child[2]);
                    par->key[0] = c[0]->key[1];
                    c[0]->popRight();   
                    
                } else if (c[1]->size == 2) {
                    DBG_OUT("2<-\n");
                    //       |                        |
                    //     par(k0)                 par(l0)
                    //     /     \                  /    \
                    //  act()  C1(l0,l1) --->  act(k0)  C1(l1)
                    //    |     / | \             / \    / \
                    //   D0    E0 E1 E2         D0 E0   E1 E2 

                    act->pushRight(par->key[0], c[1]->child[0]);
                    par->key[0] = c[1]->key[0];                       
                    c[1]->popLeft();
                }
            } else if (par->size == 2) {
                if (c[0]->size == 0 && c[1]->size == 1) {
                    DBG_OUT("3a->\n");                        
                    //          |                  |
                    //     par(k0,k1)            par(k1)
                    //     /    |    \   --->    /    \
                    //  act() C1(l0) C2      C1(k0,l0) C2
                    //    |   /  \            / | \
                    //   D0  E0  E1          D0 E0 E1 
                    
                    c[1]->pushLeft(par->key[0], act->child[0]);
                    par->popLeft();
                    __free_node(&act);
                    
                } else if (c[0]->size == 1 && c[1]->size == 0) {
                    DBG_OUT("3a<-\n");
                    //          |                  |
                    //     par(k0,k1)            par(k1)
                    //     /    |    \   --->    /    \
                    //  C0(l0) act() C2      C0(l0,k0) C2
                    //   /  \   |             / | \
                    //  D0  D1  E0          D0 D1 E0 
                    
                    c[0]->pushRight(par->key[0], act->child[0]);
                    par->addChild(1, c[0]);
                    par->popLeft();
                    __free_node(&act);
                    
                } else if (c[1]->size == 1 && c[2]->size == 0) {
                    DBG_OUT("3b<-\n");
                    //       |                    |
                    //   par(k0,k1)            par(k0)
                    //   /   |    \     --->    /    \
                    //  C0 C1(l0) act()        C0 C1(l0,k1)
                    //      / \    |                / | \
                    //     D0 D1   E0              D0 D1 E0  
                    
                    c[1]->pushRight(par->key[1], act->child[0]);
                    par->popRight();
                    __free_node(&act);
                    
                } else if (c[0]->size == 0 && c[1]->size == 2) {
                    DBG_OUT("4a->\n");
                    //         |                    |
                    //     par(k0,k1)           par(l0,k1)
                    //    /    |    \   --->   /    |    \
                    // act()C1(l0,l1)C2    act(k0) C1(l1) C2
                    //   |   / | \           / \   / \
                    //  D0  E0 E1 E2        D0 E0 E1 E2  
                    
                    act->pushRight(par->key[0], c[1]->child[0]);
                    par->key[0] = c[1]->key[0];
                    c[1]->popLeft();
                    
                } else if (c[0]->size == 2 && c[1]->size == 0) {
                    DBG_OUT("4a<-\n");
                    //           |                    |
                    //       par(k0,k1)           par(l1,k1)
                    //      /    |    \   --->   /    |    \
                    // C0(l0,l1)act() C2      C0(l0)act(k0) C2
                    //   / | \   |             / \   / \
                    // D0 D1 D2  E0           D0 D1 D2 E0   
                    
                    act->pushLeft(par->key[0] , c[0]->child[2]);
                    par->key[0] = c[0]->key[1];
                    c[0]->popRight();
                    
                } else if (c[1]->size == 2 && c[2]->size == 0) {
                    DBG_OUT("4b<-\n");
                    //         |                    |
                    //     par(k0,k1)           par(k0,l1)
                    //    /    |    \   --->   /    |    \
                    //  C0 C1(l0,l1)act()     C0 C1(l0) act(k1)
                    //       / | \   |             / \   / \
                    //     D0 D1 D2  E0           D0 D1 D2 E0   
                    
                    act->pushLeft(par->key[1], c[1]->child[2]);
                    par->key[1] = c[1]->key[1];                      
                    c[1]->popRight();
                    
                } else if (c[1]->size == 0 && c[2]->size == 1) {
                    // UNUSED CASE 
                    DBG_OUT("FUC\n");
                    assert(false);
                    
                } else if (c[1]->size == 0 && c[2]->size == 2) {
                    // UNUSED CASE
                    DBG_OUT("SUC\n");
                    assert(false);  
                    
                } else {
                    // ERROR ..
                    assert(false);
                } 
            }
        }

        return;
    }

    void clear() {
        __destructor(&root);
        __constructor();
    }
    
    inline unsigned size() {
        return _size;
    }
    
    inline bool empty() {
        return _size == 0;
    }
    
private:  
      
    inline void __constructor() {
        root = nullptr;
        _size = 0;        
    }
    
    void __destructor(Node **p) {     
        if (*p == nullptr) {
#ifdef TREE23_EXPERIMENTAL
            //for (int i=0;i<peg_node_size;i++) __free_node(&peg_node[i]);
            //for (int i=0;i<peg_key_size; i++) __free_key(&peg_key[i]);   
#endif            
            return;
        }
        switch ((*p)->size) {
            case 2: __destructor(&(*p)->child[2]);
                    __free_key(&(*p)->key[1]);
            case 1: __destructor(&(*p)->child[1]);
                    __free_key(&(*p)->key[0]);
            case 0: __destructor(&(*p)->child[0]);
        }
        __free_node(&*p); *p = nullptr;    
    }
    
    Node *findSmallerPtr(const T &val) {
        Node *it = root, *act = it;
        while (it != nullptr) {
            act = it;
            if (val < *it->key[0]) {
                it = it->child[0];
            } else if (!(*it->key[0] < val)) {
                return nullptr;
            } else if (it->size == 1 || val < *it->key[1]) {
                it = it->child[1];
            } else if (!(*it->key[1] < val)) {
                return nullptr;
            } else {
                it = it->child[2];
            }   
        }    
        return act;
    }

    Node *findPtr(const T &val) {
        Node *it = root;
        while (it != nullptr) {
            if (val < *it->key[0]) {
                it = it->child[0];
            } else if (!(*it->key[0] < val)) {
                return it;
            } else if (it->size == 1 || val < *it->key[1]) {
                it = it->child[1];
            } else if (!(*it->key[1] < val)) {
                return it;
            } else {
                it = it->child[2];
            }
        }
        return nullptr;
    }  

    inline void compare(T **a, T **b) {
        //DBG_OUT(**a << " " << **b << "\n");
        if (**b < **a) {
            T *tmp = *a;
            *a = *b;
            *b = tmp;
        }
    }   
    inline Node *__alloc_node() {
#ifdef TREE23_EXPERIMENTAL  
        return mem_Node.__new();                
#endif    
        return new Node();
    } 
    
    inline void __free_node(Node **p) {
#ifdef TREE23_EXPERIMENTAL 
        mem_Node.__delete(p);
        return;
#endif        
        delete *p;        
    }
    
    inline T *__alloc_key() {
#ifdef TREE23_EXPERIMENTAL    
        return mem_T.__new(); 
#endif    
        return new T();
    } 
   
    inline void __free_key(T **p) {
#ifdef TREE23_EXPERIMENTAL 
        mem_T.__delete(p); 
        return;
#endif        
        delete *p;        
    }
    
#ifdef TREE23_DEBUG   
public:
    void __print(Node *p) {
            if (p->child[0] != nullptr) __print(p->child[0]);
            DBG_OUT(p->key[0] << "\n");                
            if (p->child[1] != nullptr) __print(p->child[1]);
            if (p->size == 2) {
                DBG_OUT(p->key[1] << "\n");                
                if (p->child[2] != nullptr) __print(p->child[2]);
            }
    }

    void dbg(Node *p) {            
        if (p == nullptr) {
            DBG_OUT("--- null ---\n");
        } else {
            DBG_OUT("(");
            for (int i=0; i<p->size; i++) {
                if (i != 0) DBG_OUT(" ");
                DBG_OUT(*p->key[i]);
            }
            DBG_OUT(")\n");
        }
    }
    
    inline void dbg_bfs() {
        __dbg_bfs(root);
    } 
    
    void __dbg_bfs(Node *p) {
        DBG_OUT(" =^= \n");
        if (p == nullptr) return;
        std::queue<Node *> q;
        q.push(p); q.push(nullptr);
        Node *it;
        int sz = 0;
        while (q.size() > 1) {
            it = q.front(); q.pop();
            if (it == nullptr) {
                DBG_OUT(" --- \n");
                q.push(nullptr);
                continue;
            } 
            sz += it->size;
            dbg(it);
            for (int i=0; i<it->size+1; i++) {
                if (it->child[i] != nullptr) {
                    q.push(it->child[i]);
                }
            }
        }
        DBG_OUT("\tsize: " << sz << "\n");            
        DBG_OUT(" === \n");            
    }     
#endif 
    
};
