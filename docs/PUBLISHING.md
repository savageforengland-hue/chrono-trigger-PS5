# Publishing the GitHub repository and Pages site

The prepared target is `savageforengland-hue/chrono-trigger-PS5`. If publishing under a different owner or repository name, update the GitHub / Pages URLs in README.md, site/index.html and tools/build_site_docs.py first.

1. Use the empty public chrono-trigger-PS5 repository without automatically adding a README, license or .gitignore. The prepared project includes those files and preserves the original PC license.
2. Push the project to its `main` branch with your own authenticated GitHub account. Do not push local game builds or your NDS input.
3. In repository Settings → Pages → Build and deployment, choose **GitHub Actions** as the source.
4. Run the **Publish project page** workflow, or push a change to main. It generates the documentation, checks the source package and publishes the `site/` folder.
5. Check the completed deployment and both source downloads at `https://savageforengland-hue.github.io/chrono-trigger-PS5/`.

The separate **Source and host checks** workflow validates source boundaries and exercises the port without a ROM. Neither workflow needs game files or secrets.

## Preview locally

```sh
python3 -m venv .deps/site-venv
. .deps/site-venv/bin/activate
python3 -m pip install -r requirements-site.txt
python3 tools/build_site_docs.py
python3 tools/package_public_source.py
python3 tools/check_site.py
python3 -m http.server 8000 --directory site
```

Open http://localhost:8000. The repository and issue links become live after the matching GitHub repository is created. GitHub Pages deployment requires repository administration / Pages permissions, which are separate from reading or editing repository content.
