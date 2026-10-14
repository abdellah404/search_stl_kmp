
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <cstdio>
#include <stdexcept>
#include <algorithm>

using namespace std;

// ==================================================
// CLASSE 1 : Gestion du fichier
// ==================================================

class GestionTexte {
public:

    string lireFichier(string nomFichier) {
        ifstream fichier(nomFichier.c_str(), ios::binary);

        if (!fichier) {
            throw runtime_error("Impossible d'ouvrir le fichier.");
        }

        string texte(
            (istreambuf_iterator<char>(fichier)),
                     istreambuf_iterator<char>()
        );

        return texte;
    }

    void ecrireFichier(string nomFichier, string texte) {
        string temporaire = nomFichier + ".tmp";

        ofstream fichier(temporaire.c_str(), ios::binary);

        if (!fichier) {
            throw runtime_error("Erreur de creation du fichier temporaire.");
        }

        fichier << texte;
        fichier.close();

        if (!fichier) {
            remove(temporaire.c_str());
            throw runtime_error("Erreur d'ecriture.");
        }

        // Remplacer le fichier original par le fichier temporaire.
        if (rename(temporaire.c_str(), nomFichier.c_str()) != 0) {
            remove(nomFichier.c_str());

            if (rename(temporaire.c_str(), nomFichier.c_str()) != 0) {
                remove(temporaire.c_str());
                throw runtime_error("Erreur de remplacement du fichier.");
            }
        }
    }

    void enregistrerMots(string texte, string motif) {
        istringstream flux(texte);
        ofstream fichier("mots_trouves.txt");

        if (!fichier) {
            throw runtime_error("Impossible de creer mots_trouves.txt");
        }

        string mot;
        int nombre = 0;

        while (flux >> mot) {
            if (mot.find(motif) != string::npos) {
                fichier << mot << '\n';
                nombre++;

                if (nombre <= 10) {
                    cout << "Mot : " << mot << endl;
                }
            }
        }

        cout << "Nombre de mots correspondants : "
        << nombre << endl;

        cout << "Liste complete : mots_trouves.txt" << endl;
    }
};


// ==================================================
// CLASSE 2 : Recherche et remplacement
// ==================================================

class RechercheTexte {
public:

    // Construire le tableau LPS pour l'algorithme KMP.
    vector<int> calculerLPS(string motif) {
        vector<int> lps(motif.length(), 0);

        int i = 1;
        int longueur = 0;
        int taille = (int)motif.length();

        while (i < taille) {
            if (motif[i] == motif[longueur]) {
                longueur++;
                lps[i] = longueur;
                i++;
            }
            else {
                if (longueur != 0) {
                    longueur = lps[longueur - 1];
                }
                else {
                    lps[i] = 0;
                    i++;
                }
            }
        }

        return lps;
    }

    // Recherche KMP.
    vector<int> rechercherKMP(string texte, string motif) {
        vector<int> positions;

        int tailleTexte = (int)texte.length();
        int tailleMotif = (int)motif.length();

        if (tailleMotif == 0) {
            return positions;
        }

        vector<int> lps = calculerLPS(motif);

        int i = 0;
        int j = 0;

        while (i < tailleTexte) {
            if (texte[i] == motif[j]) {
                i++;
                j++;
            }

            if (j == tailleMotif) {
                positions.push_back(i - j);
                j = lps[j - 1];
            }
            else if (i < tailleTexte && texte[i] != motif[j]) {
                if (j != 0) {
                    j = lps[j - 1];
                }
                else {
                    i++;
                }
            }
        }

        return positions;
    }

    // Recherche avec la fonction find de la STL.
    vector<int> rechercherSTL(string texte, string motif) {
        vector<int> positions;

        if (motif.empty()) {
            return positions;
        }

        int position = 0;

        while (true) {
            position = (int)texte.find(motif, position);

            if (position == -1) {
                break;
            }

            positions.push_back(position);
            position++;
        }

        return positions;
    }

    // Construire le texte apres remplacement.
    string construireRemplacement(
        string texte,
        string motif,
        string remplacement,
        vector<int> positions,
        int &nombre
    ) {
        string resultat;
        int dernierePosition = 0;
        int tailleTexte = (int)texte.length();
        int tailleMotif = (int)motif.length();

        nombre = 0;

        for (int i = 0; i < (int)positions.size(); i++) {
            int position = positions[i];

            // Ignorer les occurrences qui se chevauchent.
            if (position < dernierePosition) {
                continue;
            }

            resultat.append(
                texte,
                dernierePosition,
                position - dernierePosition
            );

            resultat += remplacement;

            dernierePosition = position + tailleMotif;
            nombre++;
        }

        // Ajouter la partie restante du texte.
        resultat.append(
            texte,
            dernierePosition,
            tailleTexte - dernierePosition
        );

        return resultat;
    }

    string remplacerKMP(
        string texte,
        string motif,
        string remplacement,
        int &nombre
    ) {
        vector<int> positions = rechercherKMP(texte, motif);

        return construireRemplacement(
            texte, motif, remplacement, positions, nombre
        );
    }

    string remplacerSTL(
        string texte,
        string motif,
        string remplacement,
        int &nombre
    ) {
        vector<int> positions = rechercherSTL(texte, motif);

        return construireRemplacement(
            texte, motif, remplacement, positions, nombre
        );
    }
};


// ==================================================
// CLASSE 3 : Comparaison des performances
// ==================================================

class Benchmark {
public:

    void testerRecherche(
        string texte,
        string motif,
        RechercheTexte &recherche
    ) {
        int repetitions = 5;

        cout << "\n=== BENCHMARK RECHERCHE ===" << endl;

        for (int methode = 0; methode < 2; methode++) {
            double tempsTotal = 0;
            int nombre = 0;

            for (int i = 0; i < repetitions; i++) {
                chrono::steady_clock::time_point debut =
                chrono::steady_clock::now();

                vector<int> positions;

                if (methode == 0) {
                    positions = recherche.rechercherKMP(texte, motif);
                }
                else {
                    positions = recherche.rechercherSTL(texte, motif);
                }

                chrono::steady_clock::time_point fin =
                chrono::steady_clock::now();

                double temps =
                chrono::duration<double, milli>(fin - debut).count();

                tempsTotal += temps;
                nombre = (int)positions.size();
            }

            if (methode == 0) {
                cout << "KMP : ";
            }
            else {
                cout << "STL find : ";
            }

            cout << tempsTotal / repetitions << " ms | "
            << nombre << " occurrences" << endl;
        }

        cout << "Lecture du fichier exclue." << endl;
    }

    void testerRemplacement(
        string texte,
        string motif,
        string remplacement,
        RechercheTexte &recherche
    ) {
        int repetitions = 5;
        string resultatReference;
        int nombreReference = 0;

        cout << "\n=== BENCHMARK REMPLACEMENT ===" << endl;

        for (int methode = 0; methode < 2; methode++) {
            double tempsTotal = 0;
            int nombre = 0;

            for (int i = 0; i < repetitions; i++) {
                chrono::steady_clock::time_point debut =
                chrono::steady_clock::now();

                string resultat;

                if (methode == 0) {
                    resultat = recherche.remplacerKMP(
                        texte, motif, remplacement, nombre
                    );
                }
                else {
                    resultat = recherche.remplacerSTL(
                        texte, motif, remplacement, nombre
                    );
                }

                chrono::steady_clock::time_point fin =
                chrono::steady_clock::now();

                double temps =
                chrono::duration<double, milli>(fin - debut).count();

                tempsTotal += temps;

                if (methode == 0 && i == 0) {
                    resultatReference = resultat;
                    nombreReference = nombre;
                }
                else {
                    if (resultat != resultatReference ||
                        nombre != nombreReference) {
                        throw runtime_error(
                            "Les deux resultats sont differents."
                        );
                        }
                }
            }

            if (methode == 0) {
                cout << "KMP : ";
            }
            else {
                cout << "STL find : ";
            }

            cout << tempsTotal / repetitions << " ms | "
            << nombre << " remplacements" << endl;
        }

        cout << "Les deux methodes donnent le meme resultat." << endl;
    }
};


// ==================================================
// PROGRAMME PRINCIPAL
// ==================================================

int main(int argc, char* argv[]) {
    try {
        string nomFichier = "texte.txt";

        if (argc > 1) {
            nomFichier = argv[1];
        }

        GestionTexte gestion;
        RechercheTexte recherche;
        Benchmark benchmark;

        string texte = gestion.lireFichier(nomFichier);

        cout << "Fichier : " << nomFichier << endl;
        cout << "Taille : " << texte.length() << " octets" << endl;

        cout << "\n1. Rechercher une sous-chaine" << endl;
        cout << "2. Remplacer une sous-chaine" << endl;
        cout << "Votre choix : ";

        string choix;
        getline(cin, choix);

        if (choix != "1" && choix != "2") {
            cout << "Choix invalide." << endl;
            return 1;
        }

        string motif;

        cout << "Sous-chaine a rechercher : ";
        getline(cin, motif);

        if (motif.empty()) {
            cout << "La sous-chaine ne doit pas etre vide." << endl;
            return 1;
        }

        if (choix == "1") {
            vector<int> positions =
            recherche.rechercherKMP(texte, motif);

            cout << "\nNombre d'occurrences : "
            << positions.size() << endl;

            cout << "10 premieres positions (index 0) : ";

            int limite = min(10, (int)positions.size());

            for (int i = 0; i < limite; i++) {
                cout << positions[i] << " ";
            }

            cout << endl;

            gestion.enregistrerMots(texte, motif);

            benchmark.testerRecherche(
                texte, motif, recherche
            );
        }
        else {
            string remplacement;

            cout << "Remplacer par : ";
            getline(cin, remplacement);

            benchmark.testerRemplacement(
                texte, motif, remplacement, recherche
            );

            int nombre = 0;

            string resultat = recherche.remplacerKMP(
                texte, motif, remplacement, nombre
            );

            gestion.ecrireFichier(nomFichier, resultat);

            cout << "\n" << nombre
            << " remplacement(s) effectue(s)." << endl;

            cout << "Fichier modifie : " << nomFichier << endl;
        }
    }
    catch (const exception &e) {
        cerr << "Erreur : " << e.what() << endl;
        return 1;
    }

    return 0;
}
