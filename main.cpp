#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <list>
#include <fstream>

using namespace std;
using namespace chrono;
using namespace this_thread;

const milliseconds READ_INTERVAL(2);

/*
    T1234 Zadatak
    Vaš program na komandnoj liniji treba da učita, redom sledeće parametre:
        Kako se zove fajl iz koga se učitava ulaz.
    Fajl iz koga se učitava ulaz je binaran i sastoji se od slogova. Svaki
    slog se sastoji od:
        1 long vrednosti (vreme) i
        15 double vrednosti (niz)
    Fajl je garantnovan da je korektno formatiran i sastoji se od barem jednog
    sloga: ne morate pisati kod koji kontroliše za taj problem.
    Niti koje postoje u programu su:
        1 nit učitavač
        4 niti obrađivača
    Nit učitavač učitava jedan slog iz ulaznog fajla, čeka period obrade od
    2ms, a zatim zatraži da se sadržaj sloga obradi. Ako ima slobodnih niti
    obrađivača, to se desi odmah. Ako nema, nit učitavač ubaci nit u red
    čekanja. Red čekanja ima 7 mesta. Ako je i mesto broj 7 zauzeto, nit učitavač
    pauzira učitavanje i čeka dok se ne oslobodi mesto.
    Svako čekanje se mora prijaviti korisniku preko
    standardnog izlaza.
    Svaka nit obrađivača mora čekati vreme specificirano u long vrednosti
    (koja predstavlja broj milisekundi), a onda mora iz ulaznog niza sloga
    odrediti minimalnu vrednost. Program mora tako raditi da postoji deljena
    promenljiva koja sadrži u svakom trenutku najveći minimum nađen iz svih
    nizova. Kada obrađivač nađe minimalnu vrednost poredi je sa deljenom promenljivom
    i vrši zamenu ako je ovaj minimum veći. Ovo mora biti urađeno na način koji
    ne dozvoljava štetno preplitanje.

    Kada se obrade svi slogovi iz fajla, program ispiše maksimum svih minimuma i
    terminira program. Osim u slučaju greške, program se mora terminirati tako što
    stigne do kraja main funkcije.

    Ceo zadatak treba da stane u ovaj .cpp i kompajliraće se sa
    g++ -pthread --std=c++14 -o main main.cpp
    komandom.

    Očekivani output za input.dat koji je dat je -0.881422.

*/



struct Slog{
    milliseconds vreme;
    double niz[15];
	

};

class SinhronaVrednost{
private:
    double minimum;
    mutex m;
public:
    SinhronaVrednost() {
    	minimum = -1/0.0; 
    }
    void poredi(double d) {
        unique_lock<mutex> l(m);
        if(d > minimum) minimum = d;
    }

    double ispis() {
        unique_lock<mutex> l(m);
        return minimum;
    }
};

class RedCekanja{
private:
    list<Slog> red;
    condition_variable pun;
    condition_variable prazan;
	
    mutex m;
	
public:
    void dodaj(Slog& s) {
        unique_lock<mutex> l(m);
        if(red.size() >= 7) {
            while(red.size() >= 7) {
		
            cout<< "RED PUN"<< endl;
		
            pun.wait(l);
		
            }
        }
        red.push_back(s);
        prazan.notify_one();
    }

    Slog get() {
        unique_lock<mutex> l(m);
        if(red.size() == 0) {
            while(red.empty()) {
            cout<< "RED PRAZAN"<<endl;
            prazan.wait(l);
            }
        }
        Slog s = red.front();
        red.pop_front();
        pun.notify_one();
        return s;
    }


};

void ucitava(char* fname, RedCekanja& rc, SinhronaVrednost& v){
    ifstream fil(fname, ios::binary);
 
    while(1) {
        sleep_for(READ_INTERVAL);
        Slog s;
       fil.read((char*)&s, sizeof(Slog));


        if(fil.eof()) break;

      rc.dodaj(s);
     
    }
    fil.close();
	


    cout<< "Najveci minimum je: "<< v.ispis()<<endl;

}

void obrada(RedCekanja& rc, SinhronaVrednost& v){
    int i = 0;
    while(1) {
        Slog s = rc.get();
        sleep_for(s.vreme);

        double temp = s.niz[0];
        for(int i = 1; i< 15; i++) {
            if(s.niz[i]< temp) temp = s.niz[i];
        }
        v.poredi(temp);

    }

}

int main(int argc, char** argv){
    if(argc != 2){
        cout << "Navesti ime ulaznog fajla." << endl;
        exit(1);
    }
    RedCekanja rc;
    SinhronaVrednost sv;

    thread u(ucitava, argv[1], ref(rc), ref(sv));
    thread o[4];
    for(int i = 0; i< 4; i++) {
        o[i] = thread(obrada, ref(rc), ref(sv));
    }
    u.join();
    for(int i = 0; i< 4; i++) {
        o[i].detach();
    }


    return 0;
}
