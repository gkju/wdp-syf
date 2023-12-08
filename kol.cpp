#include <algorithm>
#include <vector>
#include <assert.h>
#include <iostream>
#include <random>
#include "kol.h"

template<typename T>
T coalesce(T a, T b) {
    return a ? a : b;
}

void clear_link(interesant *i) {
    i->i1 = i->i2 = NULL;
}

class Kolejka {
    private:
        int id;
    public:
        interesant *head = NULL, *tail = NULL;

        Kolejka(int Id) {
           this->id = Id;
        }

        int get_id() {
            return this->id;
        }

        void clear() {
            head = tail = NULL;
        }

        bool empty() {
            return !head;
        }

        void fix_nums() {
            if(head) {
                head->ostatnia_kolejka = this->id;
            }

            if(tail) {
                tail->ostatnia_kolejka = this->id;
            }
        }

        void append(interesant *nowy) {
            clear_link(nowy);
            if(!head) {
                head = tail = nowy;
                fix_nums();
                return;
            }

            (tail->i1 ? tail->i2 : tail->i1) = nowy;
            nowy->i1 = tail;
            tail = nowy;
            fix_nums();
        }

        void prepend(interesant *nowy) {
            clear_link(nowy);
            if(!head) {
                head = tail = nowy;
                fix_nums();
                return;
            }

            (head->i1 ? head->i2 : head->i1) = nowy;
            nowy->i1 = head;
            head = nowy;
            fix_nums();
        }

        interesant *pop_front() {
            interesant *old = head;
            head = coalesce(old->i1, old->i2);
            old->i1 = old->i2 = NULL;

            if(head) {
                (head->i1 == old ? head->i1 : head->i2) = NULL;
            } else {
                tail = NULL;
            }

            fix_nums();
            return old;
        }

        interesant *pop_back() {
            interesant *old = tail;
            tail = coalesce(old->i1, old->i2);
            old->i1 = old->i2 = NULL;

            if(tail) {
                (tail->i1 == old ? tail->i1 : tail->i2) = NULL;
            } else {
                head = NULL;
            }

            fix_nums();
            return old;
        }

        void register_remove(interesant& i) {
            if(this->head == &i) {
                this->head = coalesce(this->head->i1, this->head->i2);
            }
            
            if(this->tail == &i) {
                this->tail = coalesce(this->tail->i1, this->tail->i2);
            }
            fix_nums();
        }

        void join(Kolejka &b) {
            if(!b.head || !b.tail) {
                return;
            }

            if(this->empty()) {
                this->head = b.head;
                this->tail = b.tail;
                fix_nums();
                return;
            }

            (this->tail->i1 ? this->tail->i2 : this->tail->i1) = b.head;
            (b.head->i1 ? b.head->i2 : b.head->i1) = this->tail;
            this->tail = b.tail;
            fix_nums();
        }
};

class Urzad {
    private:
        int m;
        int counter = 0;
        std::vector<Kolejka> okienka;
    public:
        explicit Urzad(int M = 0) {
            this->m = M;
            this->okienka = std::vector<Kolejka>();
            for(int i = 0; i < M; i++) {
                this->okienka.push_back(Kolejka(i));
            }
        }

        int get_num() {
            return this->counter++;
        }

        int get_m() {
            return this->m;
        }
        
        Kolejka operator[](int i) const {
            return this->okienka[i];
        }

        Kolejka &operator[](int i) {
            return this->okienka[i];
        }
};

Urzad urzad;

void otwarcie_urzedu(int m) {
    urzad = Urzad(m);
}

interesant *nowy_interesant(int k) {
    interesant *nowy = (interesant*) malloc(sizeof(interesant));
    nowy->ostatnia_kolejka = k;
    nowy->numerek = urzad.get_num();
    urzad[k].append(nowy);
    return nowy;
}

int numerek(interesant *i) {
    return i->numerek;
}

interesant *obsluz(int k) {
    auto &kolejka = urzad[k];

    if(!kolejka.head) {
        return NULL;
    }

    interesant *i = kolejka.pop_front();
    return i;
}

// laczy sasiadow a ze soba
void przepnij(interesant *a) {
    if(a->i1) {
        auto &i1 = a->i1;
        (i1->i1 == a ? i1->i1 : i1->i2) = a->i2;
    }

    if(a->i2) {
        auto &i2 = a->i2;
        (i2->i1 == a ? i2->i1 : i2->i2) = a->i1;
    }
}

void zmiana_okienka(interesant *i, int k) {
    auto &kolejka = urzad[k];
    auto &stara_kolejka = urzad[i->ostatnia_kolejka];

    przepnij(i);
    stara_kolejka.register_remove(*i);
    kolejka.append(i);
}

void zamkniecie_okienka(int k1, int k2) {
    auto &kolejka1 = urzad[k1];
    auto &kolejka2 = urzad[k2];

    kolejka2.join(kolejka1);
    kolejka1.clear();
}

// znajduje w ktora strone i1 czy i2 znajduje sie b 
bool find_dir(interesant *a, interesant *b) {
    auto i1 = a->i1;
    auto i2 = a->i2;
    auto prev_i1 = a, prev_i2 = a;
    while(i1 != b && i2 != b) {
        if(i1) {
            auto old = i1;
            i1 = i1->i1 == prev_i1 ? i1->i2 : i1->i1;
            prev_i1 = old;
        }

        if(i2) {
            auto old = i2;
            i2 = i2->i1 == prev_i2 ? i2->i2 : i2->i1;
            prev_i2 = old;
        }
    }

    return i1 == b;
}

std::vector<interesant*> fast_track(interesant *in1, interesant *in2) {
    auto &kolejka = urzad[in1->ostatnia_kolejka];

    if(in1 == in2) {
        przepnij(in1);
        kolejka.register_remove(*in1);
        return std::vector<interesant*>{in1};
    }

    bool in1_dir = find_dir(in1, in2);
    bool in2_dir = find_dir(in2, in1);
    std::vector<interesant*> result;
    result.push_back(in1);
    interesant* iterator = in1_dir ? in1->i1 : in1->i2;
    interesant* prev_iterator = in1;
    while(iterator != in2) {
        result.push_back(iterator);
        auto old = iterator;
        iterator = iterator->i1 == prev_iterator ? iterator->i2 : iterator->i1;
        prev_iterator = old;
    }
    result.push_back(in2);
    auto sasiad_in1 = in1_dir ? in1->i2 : in1->i1;
    auto sasiad_in2 = in2_dir ? in2->i2 : in2->i1;
    if(!sasiad_in1) {
        (kolejka.head == in1 ? kolejka.head : kolejka.tail) = sasiad_in2;
    } else {
        (sasiad_in1->i1 == in1 ? sasiad_in1->i1 : sasiad_in1->i2) = sasiad_in2;
    }

    if(!sasiad_in2) {
        (kolejka.head == in2 ? kolejka.head : kolejka.tail) = sasiad_in1;
    } else {
        (sasiad_in2->i1 == in2 ? sasiad_in2->i1 : sasiad_in2->i2) = sasiad_in1;
    }

    return result;
}

void naczelnik(int k) {
    auto &kolejka = urzad[k];
    
    std::swap(kolejka.head, kolejka.tail);
}

std::vector<interesant*> zamkniecie_urzedu() {
    std::vector<interesant*> result;
    for(int i = 0; i < urzad.get_m(); i++) {
        auto &kolejka = urzad[i];
        while(!kolejka.empty()) {
            result.push_back(kolejka.pop_front());
        }
    }

    return result;
}