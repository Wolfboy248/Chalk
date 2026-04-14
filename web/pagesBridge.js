let bridge;
const pagesContainer = document.getElementById("pages");

let lastScrollPos = 0;

const probe = document.createElement("div");
probe.id = "probe";
document.body.appendChild(probe);

new QWebChannel(qt.webChannelTransport, function(channel) {
  bridge = channel.objects.bridge;

  window.bridge = bridge;

  bridge.setBgCol.connect(function(str) {
    document.body.style.background = str;
  });

  bridge.updatePages.connect(function(assignment) {
    lastScrollPos = window.scrollY;
    console.log("Last scroll pos: " + lastScrollPos);
    renderPages(JSON.parse(assignment));
  })

  bridge.jsReady();
});

const splitEquation = (expr) => {
  const parts = expr.split("=");
  return {
    lhs: parts[0]?.trim(),
    rhs: parts[1]?.trim()
  };
}

const isNumericExpression = (expr) => {
  return /^[\d.\s]+$/.test(expr);
}

const hasVariables = (expr) => {
  return /[a-zA-Z]/.test(expr);
}

const createPage = () => {
  const page = document.createElement("div");
  page.className = "page";

  const header = document.createElement("div");
  header.className = "page-header";

  const name = document.createElement("div");
  name.innerText = `Name: Georg Ejvind Karlsen`;

  const date = document.createElement("div");
  date.innerText = `Date: ${new Date().toLocaleDateString()}`;

  header.appendChild(name);
  header.appendChild(date);

  const content = document.createElement("div");
  content.className = "page-content";

  page.appendChild(header);
  page.appendChild(content);

  return { page, content, header };
}

const measureHeight = (el) => {
  probe.innerHTML = "";
  const clone = el.cloneNode(true);
  probe.appendChild(clone);

  const style = window.getComputedStyle(clone);

  const margin =
    parseFloat(style.marginTop) +
    parseFloat(style.marginBottom);

  const rectHeight = clone.getBoundingClientRect().height;

  console.log(rectHeight + margin);
  return rectHeight + margin;
}

const createText = (tag, text) => {
  const el = document.createElement(tag);
  el.innerText = text;
  return el;
}

const buildBlocks = (assignment) => {
  const blocks = [];

  blocks.push({
    type: "title",
    el: createText("h1", assignment.title)
  });

  assignment.tasks.forEach(task => {
    blocks.push({
      type: "task",
      id: task.id,
      el: createText("h2", task.title)
    });

    task.formulas.forEach(f => {
      if (f.explanation != "" && f.explanation != "\n") {
        blocks.push({
          type: "explanation",
          el: createText("p", f.explanation)
        })
      }

      const div = document.createElement("div");
      div.className = "formula";

      let latex = f.latex;
      console.log("Latex: " + f.latex);
      if (f.result != null && f.result != "") {
        const { lhs, rhs } = splitEquation(f.latex);
        if (rhs && hasVariables(rhs)) {
          latex += "=" + f.result;
        } else if (!rhs) {
          latex += "=" + f.result;
        }
      }

      katex.render(latex, div, { throwOnError: false });

      blocks.push({
        type: "formula",
        el: div
      });
    });
  });

  return blocks;
}

const paginate = (blocks) => {
  const pages = [];

  let page = createPage();
  let height = 0;

  const PAGE_HEIGHT = 297 * 3.78;

  pages.push(page);

  blocks.forEach(block => {
    const h = measureHeight(block.el);

    if (block.type == "title" || block.type == "task") {
      block.el.contentEditable = "true";
    }

    console.log(h + height);
    console.log(PAGE_HEIGHT);

    if (height + h > PAGE_HEIGHT) {
      page = createPage();
      pages.push(page);
      height = 0;
    }

    const clone = block.el.cloneNode(true);

    if (block.type == "title") {
      clone.addEventListener("input", () => {
        console.log("WOEOOEOEE");
        bridge.updateTitle(clone.innerText);
      })
    } else if (block.type == "task") {
      clone.addEventListener("input", () => {
        bridge.updateTaskTitle(block.id, clone.innerText)
      })
    }

    page.content.appendChild(clone);
    height += h;
  });

  return pages;
}

const renderLatex = (el, latex) => {
  window.katex.render(latex, el, {
    throwOnError: false
  });
};

const renderPages = (assignment) => {
  pagesContainer.innerHTML = "";

  const blocks = buildBlocks(assignment);
  const pages = paginate(blocks);

  pages.forEach(p => {
    pagesContainer.appendChild(p.page);
  });

  requestAnimationFrame(() => {
    requestAnimationFrame(() => {
      window.scrollTo(0, lastScrollPos);
    })
  })
}

// const renderPages = (assignment) => {
//   console.log(assignment);
//   const pages = pagesContainer;
//   pagesContainer.innerHTML = ``;
//
//   let current = createPage();
//   let currentPage = current.page;
//   let currentContent = current.content;
//   pages.appendChild(currentPage);
//
//   // return;
//
//   const addPageBreak = () => {
//     const next = createPage();
//     currentPage = next.page;
//     currentContent = next.content;
//     pages.appendChild(currentPage);
//   }
//
//   const addElement = (el) => {
//     currentContent.appendChild(el);
//
//     requestAnimationFrame(() => {
//       requestAnimationFrame(() => {
//         if (currentContent.scrollHeight > currentContent.clientHeight) {
//           currentContent.removeChild(el);
//
//           const next = createPage();
//           currentPage = next.page;
//           currentContent = next.content;
//
//           pages.appendChild(currentPage);
//           currentContent.appendChild(el);
//         }
//       })
//       // console.error(current.content.clientHeight)
//       // if (currentContent.scrollHeight > currentContent.clientHeight) {
//       //   // console.error("OMGOGMOMGOMGOGMOGMOGM");
//       //   console.error(el.innerText);
//       //   currentContent.removeChild(el);
//       //   const next = createPage();
//       //   currentPage = next.page;
//       //   currentContent = next.content;
//       //   pageNum++;
//       //   pages.appendChild(currentPage);
//       //   currentContent.appendChild(el);
//       // }
//     })
//   }
//
//   // Title
//   const titleEl = document.createElement("h1");
//   titleEl.contentEditable = "true";
//   titleEl.innerText = assignment.title;
//
//   titleEl.addEventListener("input", () => {
//     bridge.updateTitle(titleEl.innerText);
//   })
//
//   addElement(titleEl);
//
//   assignment.tasks.forEach(task => {
//     const el = document.createElement("h2");
//     el.className = "task";
//     el.contentEditable = "true";
//     el.innerText = task.title;
//
//     el.addEventListener("input", () => {
//       console.log(task.id);
//       bridge.updateTaskTitle(task.id, el.innerText);
//     })
//
//     addElement(el);
//
//     task.formulas.forEach(f => {
//       const fEl = document.createElement("div");
//       fEl.className = "formula";
//       katex.render(f.latex, fEl, {
//         throwOnError: false
//       });
//       addElement(fEl);
//     })
//   });
// }
