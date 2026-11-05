# GRFTC

Petite application Windows native en C++17, avec contrôles Win32 et rendu GDI, sans framework externe.

## Fonctionnalités

- Cinq champs : Puissance, Température, FDH, Débit relatif, FDHE1
- Décimales avec point ou virgule, nombres négatifs et notation scientifique acceptés
- Calcul immédiat `RFTC = Puissance - Température`, isolé dans `src/numeric.h`
- Une saisie vide, invalide ou hors limites suspend l'affichage du résultat
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

1. Entrer Puissance = 100 et Température = 25 : `RFTC=75`
2. Entrer Puissance = 10,5 et Température = -2 : `RFTC=12.5`
3. Effacer un champ ou écrire `abc` : saisie invalide ; rétablir un nombre : résultat rétabli
4. Parcourir les cinq champs avec Tab et Maj+Tab
5. Dans le canvas, maintenir le bouton gauche et tracer verticalement : la courbe noire et son aire rouge suivent la souris
6. Repasser en sens inverse : seules les hauteurs parcourues sont modifiées
7. Déplacer rapidement la souris : le tracé reste continu entre les positions reçues
8. Sortir du canvas bouton enfoncé, puis relâcher : le dessin reste borné au repère et s’arrête au relâchement
9. Redimensionner ou masquer puis réafficher la fenêtre : le dessin est conservé
10. Cliquer dans la marge blanche : aucun tracé ne commence

La compilation et l'exécution graphique sous Windows restent à valider avant fusion.

## Coordonnées de la courbe

Le repère utilise provisoirement `x` et `y` entre 0 et 1 : origine en bas à gauche, `y` croissant vers le haut. La courbe possède 1 025 échantillons, initialement à `x=0`. Tracer remplace les abscisses des hauteurs parcourues ; un nouveau clic ne relie pas deux traits distincts. Les coordonnées sont bornées au repère quand la souris en sort pendant un tracé. Une perte de capture ou un redimensionnement interrompt le geste.

Les tests `curve` vérifient l’interpolation dans les deux sens, le remplacement à hauteur constante et les limites du repère. Le calcul RFTC reste indépendant du dessin.
