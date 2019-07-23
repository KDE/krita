Krita Flatpak
-------------

1. add the flathub repository:

`$ flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo`

2. install the KDE SDK from flathub:

`$ flatpak install flathub org.kde.Sdk`

3. compile krita and install it into a local repository:

`$ flatpak-builder --repo=repo_dir --force-clean build_dir org.kde.krita.yaml`

4. export krita from the local repository to a bundle:

`$ flatpak build-bundle repo_dir krita--x86_64.flatpak org.kde.krita master`

5. install the bundle:

`$ flatpak install krita-x86_64.flatpak` 
