# GRFTC

Petite application Windows native en C++17, avec contrôles Win32 et rendu GDI, sans framework externe.

## Fonctionnalités

- Liste de configurations lue depuis `configurations.txt`
- Six champs : Puissance, Température, Pression, FDH, Débit relatif, FDHE1
- Décimales avec point ou virgule, nombres négatifs et notation scientifique acceptés
- Calcul immédiat `RFTC = Puissance - Température`, isolé dans `src/RFTC.cpp`
- Une saisie vide, invalide ou hors limites suspend l'affichage du résultat
- Un message d’erreur précise la contrainte à la sortie d’un champ invalide
- Navigation par Tab et Maj+Tab
- Canvas blanc GDI : dessin de `x = f(y)` au bouton gauche et remplissage rouge jusqu’à `x=0`
- Une seule abscisse par hauteur ; les mouvements rapides sont interpolés
- Dessin conservé lors du redimensionnement ; double tampon pour limiter le scintillement

## Compilation sous Windows

Installer Visual Studio 2022 avec « Développement Desktop en C++ » et CMake, puis :

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\build\Release\GRFTC.exe
```

Le runtime MSVC est lié statiquement : pas de redistribuable C++ à installer.
Les seules bibliothèques graphiques utilisées sont celles de Windows (`user32`, `gdi32`).

## Vérification manuelle

1. Au démarrage : CONFIG1, puissance 100, température 290, pression 155, FDH 1.4, débit relatif 100 et FDHE1 1.00 ; `RFTC=-190`
2. Entrer Puissance = 150 et Température = 300 : `RFTC=-150`
3. Entrer `abc`, laisser un champ vide ou dépasser ses limites puis quitter le champ par Tab ou en cliquant ailleurs : message d’erreur ; le résultat reste suspendu jusqu’à correction
4. Parcourir la liste et les six champs avec Tab et Maj+Tab ; vérifier les bornes inclusives du tableau ci-dessous
5. Dans le canvas, maintenir le bouton gauche et tracer verticalement : la courbe noire et son aire rouge suivent la souris
6. Repasser en sens inverse : seules les hauteurs parcourues sont modifiées
7. Déplacer rapidement la souris : le tracé reste continu entre les positions reçues
8. Sortir du canvas bouton enfoncé, puis relâcher : le dessin reste borné au repère et s’arrête au relâchement
9. Redimensionner ou masquer puis réafficher la fenêtre : le dessin est conservé
10. Cliquer dans la marge blanche : aucun tracé ne commence

La compilation et les nouvelles interactions graphiques sous Windows restent à valider avant fusion.

## Coordonnées de la courbe

Le repère utilise provisoirement `x` et `y` entre 0 et 1 : origine en bas à gauche, `y` croissant vers le haut. La courbe possède 1 025 échantillons, initialement à `x=0`. Tracer remplace les abscisses des hauteurs parcourues ; un nouveau clic ne relie pas deux traits distincts. Les coordonnées sont bornées au repère quand la souris en sort pendant un tracé. Une perte de capture ou un redimensionnement interrompt le geste.

Les tests `curve` vérifient l’interpolation dans les deux sens, le remplacement à hauteur constante et les limites du repère. Le calcul RFTC reste indépendant du dessin.

## Configurations

Modifier `configurations.txt` en UTF-8, avec un nom par ligne, pour ajouter, supprimer ou renommer des configurations. Les lignes vides et les lignes commençant par `#` sont ignorées. La première configuration est sélectionnée au démarrage.

Le programme lit ce fichier **à côté de GRFTC.exe**, quel que soit le dossier courant. Pour tester sans recompiler, modifier `build/Release/configurations.txt`, puis relancer l’application. Pour conserver la modification lors des prochaines compilations, modifier également le fichier à la racine du dépôt : chaque build copie ce dernier près de l’exécutable. Un fichier absent, vide ou mal encodé provoque un message explicite au démarrage.

## Paramètres et limites

| Champ | Valeur initiale | Minimum inclus | Maximum inclus |
| --- | --- | --- | --- |
| Puissance | 100 | 10 | 200 |
| Température | 290 | 250 | 350 |
| Pression | 155 | 130 | 170 |
| FDH | 1.4 | 1.0 | 5.0 |
| Débit relatif | 100 | Non imposé | Non imposé |
| FDHE1 | 1.00 | 1.00 | 1.10 |

Les bornes sont regroupées dans `src/limites_statepoint.h` : les modifier puis recompiler. Les valeurs initiales sont dans `src/statepoint.h`. Tous les champs doivent contenir un double fini ; le point, la virgule et la notation scientifique sont acceptés. Une saisie invalide est conservée pour correction, avec un message à la perte du focus et aucun calcul sur des paramètres invalides.

## Fonction RFTC

Déclaration dans `src/RFTC.h`, définition dans `src/RFTC.cpp` :

```cpp
double RFTC(char* nom_configuration, double puissance, double temperature,
            double pression, double fdh, double debit, double fhde1);
```

Le nom sélectionné est transmis en UTF-8 avec les six valeurs validées dans cet ordre. La formule reste provisoirement `puissance - temperature` ; tous les arguments sont disponibles pour le futur calcul.

Les tests `numeric`, `statepoint` et `curve` couvrent la conversion numérique, la formule provisoire, les valeurs initiales, les limites inclusives et leur dépassement, la lecture d’une liste de configurations de longueur variable et l’interpolation de la courbe.
