# GRFTC

Petite application Windows native en C++17, avec contrôles Win32 et rendu GDI, sans framework externe.

## Fonctionnalités

- Liste de configurations lue depuis `configurations.txt`
- Six champs : Puissance, Température, Pression, FDH, Débit relatif, FDHE1
- Décimales avec point ou virgule, nombres négatifs et notation scientifique acceptés
- Calcul immédiat selon la configuration, isolé dans `src/RFTC.cpp`
- Une saisie vide, invalide ou hors limites suspend l'affichage du résultat
- À la sortie d’un champ invalide : message d’erreur, puis rétablissement de la dernière saisie validée pour ce champ
- Navigation par Tab et Maj+Tab
- Canvas blanc GDI : dessin de `x = f(y)` au bouton gauche et remplissage rouge jusqu’à `x=0`
- Une seule abscisse par hauteur ; les mouvements rapides sont interpolés
- Dessin conservé lors du redimensionnement ; double tampon pour limiter le scintillement
- Courbe initiale programmable dans `src/initial_curve.cpp`
- AO affiché sous RFTC en pourcentage, avec une décimale, et actualisé pendant le dessin

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
3. Saisir une puissance valide (par exemple 120), quitter le champ, puis saisir `abc`, vider le champ ou dépasser ses limites : après le message d’erreur, 120 est rétabli et le résultat recalculé
4. Parcourir la liste et les six champs avec Tab et Maj+Tab ; vérifier les bornes inclusives du tableau ci-dessous
5. Dans le canvas, maintenir le bouton gauche et tracer verticalement : la courbe noire et son aire rouge suivent la souris
6. Repasser en sens inverse : seules les hauteurs parcourues sont modifiées
7. Déplacer rapidement la souris : le tracé reste continu entre les positions reçues
8. Sortir du canvas bouton enfoncé, puis relâcher : le dessin reste borné au repère et s’arrête au relâchement
9. Redimensionner ou masquer puis réafficher la fenêtre : le dessin est conservé
10. Cliquer dans la marge blanche : aucun tracé ne commence

La compilation et les nouvelles interactions graphiques sous Windows restent à valider avant fusion.

## Coordonnées et courbe initiale

Le repère utilise `x` entre 0 et 2 et `y` entre 0 et 1 : origine en bas à gauche, `y` croissant vers le haut. Les limites sont centralisées dans `src/limites_graph.h` et les axes affichent leurs valeurs. L’abscisse de gauche reste 0 pour représenter l’aire jusqu’à l’axe `x=0`.

Modifier la fonction `InitialCurve(double y)` dans `src/initial_curve.cpp`, puis recompiler, pour choisir la forme au démarrage. Elle contient actuellement :

```cpp
return 0.2 + 1.5 * std::sin(2.0 * pi * y);
```

La courbe possède 1 025 échantillons, rangés du haut vers le bas. La fonction est évaluée avec `y=1` en haut et `y=0` en bas. **Les valeurs négatives ou supérieures à 2 sont bornées à la zone visible** ; une valeur non finie est remplacée par 0. Cette convention est nécessaire ici car le sinus demandé descend jusqu’à -1.3.

Tracer remplace les abscisses des hauteurs parcourues ; un nouveau clic ne relie pas deux traits distincts. Les coordonnées sont bornées au repère quand la souris en sort pendant un tracé. Une perte de capture ou un redimensionnement interrompt le geste. Les valeurs physiques sont conservées au redimensionnement.

## AO

Le fichier `src/ao.cpp` définit `double AO(const Curve& curve)`, qui renvoie :

```text
(aire de la moitié haute - aire de la moitié basse) / aire totale
```

Les aires sont celles de la courbe bornée affichée en rouge. Elles sont intégrées par trapèzes, avec une séparation à mi-hauteur (`y=0.5` pour les limites actuelles). Si l’aire totale vaut 0, AO vaut 0. Le calcul renvoie une fraction ; l’interface la multiplie par 100 et affiche une seule décimale avec une virgule, par exemple `AO=-98,5 %` au démarrage avec la formule actuelle.

AO est mis à jour à chaque modification de la courbe, indépendamment des paramètres RFTC. Une courbe symétrique donne 0 %, une aire uniquement dans la moitié haute donne +100 %, et une aire uniquement dans la moitié basse donne -100 %.

Vérifications manuelles : au lancement, contrôler la forme sinusoïdale bornée et `AO=-98,5 %`, puis dessiner dans les deux moitiés et vérifier l’actualisation d’AO. Redimensionner la fenêtre doit conserver la forme et AO. Pour tester une autre initialisation, essayer `return 1.0;` dans `InitialCurve` (AO=0,0 %), puis `return 0.0;` (AO=0,0 % également).

Les tests `ao` couvrent l’aire nulle, une courbe constante, des triangles de résultat analytique ±50 %, les cas ±100 %, l’orientation de la courbe initiale et la conversion entre pixels normalisés et abscisses physiques. Le calcul RFTC reste indépendant du dessin.

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
| Débit relatif | 100 | 30 | 110 |
| FDHE1 | 1.00 | 1.00 | 1.10 |

Les bornes sont regroupées dans `src/limites_statepoint.h` : les modifier puis recompiler. Les valeurs initiales sont dans `src/statepoint.h`. Tous les champs doivent contenir un double fini ; le point, la virgule et la notation scientifique sont acceptés. Une saisie invalide déclenche un message à la perte du focus. Après fermeture du message, la dernière saisie validée à la sortie de ce champ est rétablie (ou sa valeur initiale si aucune nouvelle valeur n’a été validée). Le résultat est alors recalculé.

## Fonction RFTC

Déclaration dans `src/RFTC.h`, définition dans `src/RFTC.cpp` :

```cpp
double RFTC(char* nom_configuration, double puissance, double temperature,
            double pression, double fdh, double debit, double fhde1);
```

Le nom sélectionné est transmis en UTF-8 avec les six valeurs validées dans cet ordre. La comparaison du nom est exacte et sensible à la casse :

- `CONFIG1` : `puissance - temperature`
- Tout autre nom : `puissance * fdh - temperature + pression`

Avec les valeurs initiales, CONFIG1 donne `-190` et les autres configurations donnent `5`. Le débit et FDHE1 restent transmis à la fonction, sans intervenir dans ces formules.

Vérifier également le rétablissement de 100 après une première saisie invalide au démarrage, la conservation des virgules dans une saisie validée, les débits 30 et 110 acceptés, et 29.99 et 110.01 refusés.

Les tests `numeric`, `statepoint`, `curve` et `ao` couvrent la conversion numérique, les deux formules, les valeurs initiales, les limites inclusives et leur dépassement, la lecture d’une liste de configurations de longueur variable et l’interpolation de la courbe.
