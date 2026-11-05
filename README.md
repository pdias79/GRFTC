# GRFTC

Petite application Windows native en C++17, avec contrôles Win32 et rendu GDI, sans framework externe.

## Première étape

- Cinq champs : Puissance, Température, FDH, Débit relatif, FDHE1
- Décimales avec point ou virgule, nombres négatifs et notation scientifique acceptés
- Calcul immédiat `RFTC = Puissance - Température`, isolé dans `src/numeric.h`
- Une saisie vide, invalide ou hors limites suspend l'affichage du résultat
- Navigation par Tab et Maj+Tab
- Zone blanche à droite réservée au dessin de `x = f(y)` dans la prochaine PR

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
5. Redimensionner la fenêtre : résultat en bas à gauche et panneau droit blanc

La compilation et l'exécution graphique sous Windows restent à valider avant fusion.
