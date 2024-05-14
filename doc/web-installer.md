## Working with web-installer

Web installer allows flashing ESP firmware right from the browser and then provision WiFi credentials to the board over Serial (using [Improv-WiFi](https://github.com/jnthas/Improv-WiFi-Library) library). The current version of the snapclient has a few common configurations pre-built that allow flashing those selected boards using provided installer.

### Installer Development

Web-installer development looks like this

- Enter the [/installer](/installer) folder. Run `npm install` once to pull the necessary packages
- Run `npm run dev` to fire up the localhost instance of the installer. The browser window will open automatically. You now may change files within the `installer` folder and the page at localhost will automatically rebuild and reload in the browser window.
- As soon as you are ready to commit, run `npm run build` to generate a bundled version of the page in the `docs` folder. Now you may commit both the development and built version of the page.
- In your GitHub repo settings you need to configure the docs source as follows, to automatically publish files from the `docs` folder to the `*.github.io` site. Note that you may change the branch to the feature branch when you are in development (to validate on the actual github.io site)
![image](https://github.com/sonocotta/esparagus-snapclient/assets/5459747/6c4fe64f-eb74-4c3e-a70d-2147e14120a9)
- Don't forget to change the above settings when your branch is merged, as pages will be automatically removed if the source branch no longer exists

### Snapcast development and installer

Whenever you change the snapclient code or board configuration, you may want to update binaries that are included in the web installer.

The process looks as follows

- Assuming you're created and work in the feature branch `features/some-cool-feature` (or any other name starts with `features`)
- Whenever you change the code and push it to the branch, GitHub action will fire up and build pre-defined configurations. It will **commit** build artifacts to the same branch
![image](https://github.com/sonocotta/esparagus-snapclient/assets/5459747/2300418f-cbbf-448f-9767-030c6d89a3b7)
- You need to sync with the feature branch (`git pull`) to test binaries locally.
- As soon as you finish, your pull request will include both code changes and binary changes

This is not the most elegant approach, but the problem is both code and releases are served from github.com domain. Github pages (and thus the installer) are served from github.io domain. JS code cannot fetch files cross-domain unless CORS headers are present, and they are not in fact.
