// Populate the sidebar
//
// This is a script, and not included directly in the page, to control the total size of the book.
// The TOC contains an entry for each page, so if each page includes a copy of the TOC,
// the total size of the page becomes O(n**2).
class MDBookSidebarScrollbox extends HTMLElement {
    constructor() {
        super();
    }
    connectedCallback() {
        this.innerHTML = '<ol class="chapter"><li class="chapter-item expanded "><a href="01-Intro.html"><strong aria-hidden="true">1.</strong> Adrenaline</a></li><li class="chapter-item expanded affix "><li class="part-title">User Manual</li><li class="chapter-item expanded "><a href="02-Install.html"><strong aria-hidden="true">2.</strong> Installation and Update</a></li><li class="chapter-item expanded "><a href="03-AdrenalineMenu.html"><strong aria-hidden="true">3.</strong> Adrenaline Menu</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="03-AdrenalineMenu/01-SaveState.html"><strong aria-hidden="true">3.1.</strong> Save State</a></li><li class="chapter-item expanded "><a href="03-AdrenalineMenu/02-AdrenalineSettings.html"><strong aria-hidden="true">3.2.</strong> Adrenaline App Settings</a></li></ol></li><li class="chapter-item expanded "><a href="04-HomebrewSupport.html"><strong aria-hidden="true">4.</strong> PSP Homebrew Support</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="04-HomebrewSupport/01-Plugin.html"><strong aria-hidden="true">4.1.</strong> PSP Plugins</a></li><li class="chapter-item expanded "><a href="04-HomebrewSupport/02-PluginConfigFormat.html"><strong aria-hidden="true">4.2.</strong> Plugin Configuration Format</a></li><li class="chapter-item expanded "><a href="04-HomebrewSupport/03-NotablePlugins.html"><strong aria-hidden="true">4.3.</strong> Notable Plugins</a></li></ol></li><li class="chapter-item expanded "><a href="05-PSPPlayback.html"><strong aria-hidden="true">5.</strong> PSP Playback</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="05-PSPPlayback/01-CompressedIsoFormats.html"><strong aria-hidden="true">5.1.</strong> Compressed ISO Formats</a></li><li class="chapter-item expanded "><a href="05-PSPPlayback/02-Inferno.html"><strong aria-hidden="true">5.2.</strong> Inferno Driver</a></li><li class="chapter-item expanded "><a href="05-PSPPlayback/03-March33.html"><strong aria-hidden="true">5.3.</strong> March33 Driver</a></li><li class="chapter-item expanded "><a href="05-PSPPlayback/04-Galaxy.html"><strong aria-hidden="true">5.4.</strong> Galaxy Controller</a></li></ol></li><li class="chapter-item expanded "><a href="06-PS1Playback.html"><strong aria-hidden="true">6.</strong> PS1 Playback</a></li><li class="chapter-item expanded "><a href="07-VSHMenu.html"><strong aria-hidden="true">7.</strong> VSH Menu</a></li><li class="chapter-item expanded "><a href="08-CfwConfiguration.html"><strong aria-hidden="true">8.</strong> Adrenaline Custom Firmware Settings</a></li><li class="chapter-item expanded "><a href="09-RecoveryMenu.html"><strong aria-hidden="true">9.</strong> Recovery Menu</a></li><li class="chapter-item expanded "><a href="10-XmbCfwMenus.html"><strong aria-hidden="true">10.</strong> XMB CFW Menus</a></li><li class="chapter-item expanded affix "><li class="part-title">Developer Manual</li><li class="chapter-item expanded "><a href="11-Contributing.html"><strong aria-hidden="true">11.</strong> Contributing</a></li><li class="chapter-item expanded "><a href="12-UsingCFWAPI.html"><strong aria-hidden="true">12.</strong> Using CFW APIs</a></li><li class="chapter-item expanded affix "><a href="XX-Glossary.html">Glossary</a></li><li class="chapter-item expanded affix "><a href="XX-CompatIssues.html">Compatibility Issues</a></li><li class="chapter-item expanded affix "><a href="XX-FAQ.html">Fun Facts and FAQ</a></li><li class="chapter-item expanded affix "><a href="XX-Changelog.html">Changelog</a></li><li class="chapter-item expanded affix "><a href="XX-Credits.html">Credits</a></li></ol>';
        // Set the current, active page, and reveal it if it's hidden
        let current_page = document.location.href.toString().split("#")[0].split("?")[0];
        if (current_page.endsWith("/")) {
            current_page += "index.html";
        }
        var links = Array.prototype.slice.call(this.querySelectorAll("a"));
        var l = links.length;
        for (var i = 0; i < l; ++i) {
            var link = links[i];
            var href = link.getAttribute("href");
            if (href && !href.startsWith("#") && !/^(?:[a-z+]+:)?\/\//.test(href)) {
                link.href = path_to_root + href;
            }
            // The "index" page is supposed to alias the first chapter in the book.
            if (link.href === current_page || (i === 0 && path_to_root === "" && current_page.endsWith("/index.html"))) {
                link.classList.add("active");
                var parent = link.parentElement;
                if (parent && parent.classList.contains("chapter-item")) {
                    parent.classList.add("expanded");
                }
                while (parent) {
                    if (parent.tagName === "LI" && parent.previousElementSibling) {
                        if (parent.previousElementSibling.classList.contains("chapter-item")) {
                            parent.previousElementSibling.classList.add("expanded");
                        }
                    }
                    parent = parent.parentElement;
                }
            }
        }
        // Track and set sidebar scroll position
        this.addEventListener('click', function(e) {
            if (e.target.tagName === 'A') {
                sessionStorage.setItem('sidebar-scroll', this.scrollTop);
            }
        }, { passive: true });
        var sidebarScrollTop = sessionStorage.getItem('sidebar-scroll');
        sessionStorage.removeItem('sidebar-scroll');
        if (sidebarScrollTop) {
            // preserve sidebar scroll position when navigating via links within sidebar
            this.scrollTop = sidebarScrollTop;
        } else {
            // scroll sidebar to current active section when navigating via "next/previous chapter" buttons
            var activeSection = document.querySelector('#sidebar .active');
            if (activeSection) {
                activeSection.scrollIntoView({ block: 'center' });
            }
        }
        // Toggle buttons
        var sidebarAnchorToggles = document.querySelectorAll('#sidebar a.toggle');
        function toggleSection(ev) {
            ev.currentTarget.parentElement.classList.toggle('expanded');
        }
        Array.from(sidebarAnchorToggles).forEach(function (el) {
            el.addEventListener('click', toggleSection);
        });
    }
}
window.customElements.define("mdbook-sidebar-scrollbox", MDBookSidebarScrollbox);
