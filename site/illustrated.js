(() => {
    "use strict";

    let ill = {
        anchors: {}
    };

    // viewports etc

    ill.elementIsVisible = (el) => {
        let rect = el.getBoundingClientRect(),
            viewHeight = Math.max(document.documentElement.clientHeight, window.innerHeight);
        return !(rect.bottom < 0 || rect.top - viewHeight >= 0);
    };

    ill.ensureElementInView = (el) => {
        if (!ill.elementIsVisible(el)) {
            el.scrollIntoView({behavior: "smooth"});
        }
    };

    // events

    ill.unselectAllRecords = () => {
        document.querySelectorAll(
            ".illustrated .record.selected, .illustrated .calculation.selected"
        ).forEach(el => {
            el.classList.remove("selected");
        });
    };

    ill.toggleRecord = (element, event) => {
        let selected = element.classList.contains("selected");
        ill.unselectAllRecords();
        if (!selected) {
            element.classList.add("selected");
            if (event) { ill.changeHash(element.dataset.anchor); }
        } else {
            ill.closeAllCode();
            if (event) { ill.changeHash(""); }
        }
        ill.cancel(event);
        ill.ensureElementInView(element);
    };

    ill.selectRecord = (element, event) => {
        ill.unselectAllRecords();
        element.classList.add("selected");
        if (event) { ill.changeHash(element.dataset.anchor); }
        ill.cancel(event);
        ill.ensureElementInView(element);
    };

    ill.showCode = (element, event) => {
        element.classList.add("show");
        ill.cancel(event);
    };

    ill.closeAllCode = () => {
        document.querySelectorAll("codesample.show").forEach(el => {
            el.classList.remove("show");
        });
    };

    ill.getAncestorAnchor = (el) => {
        while (el && !el.dataset.anchor) {
            el = el.parentElement;
        }
        return el?.dataset?.anchor;
    };

    ill.toggleAnnotate = (el, event) => {
        let anchor = ill.getAncestorAnchor(el);
        if (el.classList.toggle("annotate")) {
            anchor = `${anchor}/annotated`;
        }
        if (event) { ill.changeHash(anchor); }
        ill.cancel(event);
    };

    ill.cancel = (event) => {
        if (event) { event.stopPropagation(); }
    };

    // injections

    ill.addShowCode = (el) => {
        el.innerHTML = document.getElementById("showCodeTmpl").innerHTML + el.innerHTML;
    };

    function htmlToElement(html) {
        let outer = document.createElement("template");
        outer.innerHTML = html.trim();
        return outer.content.firstChild;
    }

    ill.addAnchors = (record) => {
        let label = record.getElementsByClassName("rec-label");
        label = label && label[0].textContent;
        let count = 1;
        if (label) {
            label = label.toLowerCase().replaceAll(/[^a-z\d]/g, "-");
            while (ill.anchors[label]) {
                label = label.replaceAll(/-\d+$/g, "");
                label = `${label}-${++count}`;
            }
            record.dataset.anchor = label;
            ill.anchors[label] = record;
            ill.anchors[`${label}/annotated`] = record;
            record.insertBefore(
                htmlToElement(`<a class="no-show" href="#${label}/annotated"></a>`), record.firstChild);
            record.insertBefore(
                htmlToElement(`<a class="no-show" href="#${label}"></a>`), record.firstChild);
        }
    };

    ill.resolveHash = () => {
        let hash = window.location.hash.replace(/^#/, "");
        const rec = ill.anchors[hash];
        if (!rec) {
            return;
        }
        ill.selectRecord(rec, null);
        if (hash.endsWith("/annotated")) {
            const b = rec.getElementsByClassName("annotate-toggle");
            if (b && b.length) {
                ill.toggleAnnotate(b[0].parentElement);
            }
        }
    };

    ill.addToggleAnnotations = (record) => {
        let expl = record.querySelector(".rec-explanation");
        let copy = document.getElementById("annotateTmpl").cloneNode(true);
        // always true (IDE warning)
        if (copy instanceof Element) { expl.insertAdjacentElement("afterend", copy); }
        copy.addEventListener("click", (e) => { ill.toggleAnnotate(record, e); });
    };

    ill.injectLabels = () => {
        let els = document.querySelectorAll(".string > .explanation, .decryption > .explanation");
        els.forEach(expl => {
            let label = expl.parentNode.querySelector(".label"),
                h4 = document.createElement("h4");
            h4.appendChild(document.createTextNode(label.textContent));
            expl.insertAdjacentElement("afterbegin", h4);
        });
    };

    ill.toggleHeaderProtection = () => {
        let els = document.querySelectorAll(".bytes.protected");
        els.forEach(el => { el.classList.toggle("hp-disabled"); });
        els = document.querySelectorAll(".bytes.unprotected");
        els.forEach(el => { el.classList.toggle("hp-disabled"); });
        let btns = document.querySelectorAll("button.hp-toggle");
        const b = btns[0];
        const text = b.textContent === b.dataset.declbl ? b.dataset.enclbl : b.dataset.declbl;
        btns.forEach(el => { el.textContent = text; });
    };

    ill.printMode = () => {
        // add printmode css
        let inject = document.createElement("link");
        inject.setAttribute("rel", "stylesheet");
        inject.setAttribute("href", "printmode.css");
        document.head.appendChild(inject);
        // open everything up
        document.querySelectorAll(".record, .calculation").forEach(el => {
            el.classList.add("selected");
            el.classList.add("annotate");
        });
        document.querySelectorAll("codesample").forEach(el => {
            el.classList.add("show");
        });
        document.querySelectorAll("*").forEach(el => {
            el.onclick = null;
        });
    };

    ill.changeHash = (hash) => {
        let href = window.location.href.replace(/#.*/, "");
        if (hash) {
            window.history.replaceState({}, "", `${href}#${hash}`);
        } else {
            window.history.replaceState({}, "", `${href}`);
        }
    };


    window.onload = () => {
        document.querySelectorAll(".record, .calculation").forEach(el => {
            ill.addAnchors(el);
            el.onclick = (event) => {
                if (el === event.target && event.offsetY < 60) {
                    ill.toggleRecord(el, event);
                } else {
                    ill.selectRecord(el, event);
                }
            };
        });
        document.querySelectorAll(".rec-label").forEach(el => {
            el.onclick = (event) => {
                ill.toggleRecord(el.parentNode, event);
            };
        });
        document.querySelectorAll("button.hp-toggle").forEach(el => {
            el.dataset.enclbl = "Encrypt record numbers";
            el.dataset.declbl = "Decrypt record numbers";
            el.innerText = el.dataset.declbl;
            el.onclick = (event) => {
                ill.toggleHeaderProtection();
                ill.cancel(event);
            };
        });
        document.querySelectorAll(".record").forEach(el => {
            ill.addToggleAnnotations(el);
        });
        document.querySelectorAll("codesample").forEach(el => {
            ill.addShowCode(el);
            el.addEventListener("click", (event) => { ill.showCode(el, event); });
        });
        document.querySelectorAll(".bytes.protected").forEach(el => {
            el.setAttribute("title", "encrypted header number");
        });
        ill.resolveHash();
        ill.injectLabels();
    };

    window.onkeyup = (e) => {
        let els;
        if (e.key === "Escape" || e.key === "Esc") {
            els = document.querySelectorAll(".record.annotate");
            if (els.length) {
                els.forEach(rec => { ill.toggleAnnotate(rec, e); });
            } else {
                ill.unselectAllRecords();
                ill.changeHash("");
            }
        }
    };

    window.ill = ill;
})();
