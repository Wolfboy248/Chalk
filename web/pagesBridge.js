"use strict";

let bridge = null;
let prevKeys = null;
let lastScrollY = 0;

const blockRefs = new Map();

const pagesContainer = document.getElementById("pages");

let lastScrollPos = 0;

const probe = document.createElement("div");
probe.id = "probe";
document.body.appendChild(probe);

const debounce = (fn, delay = 400) => {
  let timer;
  return (...args) => {
    clearTimeout(timer);
    timer = setTimeout(() => fn(...args), delay);
  };
};

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

function easeInOutCubic(x) {
  return x < 0.5 ? 4 * x * x * x : 1 - Math.pow(-2 * x + 2, 3) / 2;
}

function scrollToSmoothly(pos, time) {
  var startPos = window.pageYOffset;
  var start = null;
  if(time == null) time = 500;
  pos = +pos;
  time = +time;
  window.requestAnimationFrame(function step(currentTime) {
    start = !start ? currentTime : start;
    var progress = currentTime - start;

    let t = Math.min(progress / time, 1);
    let eased = easeInOutCubic(t);

    let y = startPos + (pos - startPos) * eased;

    window.scrollTo(0, y);

    if (progress < time) {
        window.requestAnimationFrame(step);
    } else {
        window.scrollTo(0, pos);
    }
  });
}

const scrollToTask = (task) => {
  console.log("Scrolling to task: " + task.title);
  const el = document.getElementById(`task:${task.id}`);
  if (!el) return;

  const top = el.getBoundingClientRect().top + window.pageYOffset;

  scrollToSmoothly(top, 200);
  // el.scrollIntoView({
  //   behavior: "smooth",
  //   block: "start",
  // });
}

// Formula latex
const buildLatex = (f) => {
  let latex = f.latex;
  // console.log("Latex: " + f.latex);
  if (f.result != null && f.result != "") {
    let result = f.result;

    console.log(latex + ": " + f.hideAnswer);

    const { lhs, rhs } = splitEquation(f.latex);
    if (rhs && hasVariables(rhs) && f.error != "__DO_NOT_SHOW__" && !f.hideAnswer) {
      if (f.isAnswer) {
        latex += "=\\underline{\\underline{" + result + "}}";
      } else {
        latex += "=" + result;
      }
    } else if (!rhs) {
      if (f.isAnswer && !f.hideAnswer) {
        latex += "=\\underline{\\underline{" + result + "}}";
      } else {
        latex += "=" + result;
      }
    }
  } else {
    if (f.isAnswer) {
      latex = "\\underline{\\underline{" + latex + "}}";
    }
  }
  //   const rhs = f.latex.split("=")[1]?.trim();
  //   const hasVar = (s) => {/[a-zA-Z]/.test(s)};
  //   if (!rhs || hasVar(rhs)) {
  //     let result = f.result.replace(" ", "\\  ");
  //     latex += (f.isAnswer
  //       ? "=\\underline{\\underline{" + f.latex + "}}"
  //       : "=") + result;
  //   }
  // } else if (f.isAnswer) {
  //   latex = "\\underline{\\underline{" + latex + "}}";
  // }
  return latex;
}

const buildBlocks = (assignment) => {
  const blocks = [];

  blocks.push({
    type: "title",
    text: assignment.title
  });

  for (const task of assignment.tasks) {
    // console.error("ID: " + task.id);
    blocks.push({ type: "task", id: task.id, text: task.title });

    for (const f of task.formulas) {
      if (f.explanation && f.explanation !== "" && f.explanation !== "\n") {
        blocks.push({ type: "explanation", formulaId: f.id, text: f.explanation });
      }
      blocks.push({ type: "formula", formulaId: f.id, latex: buildLatex(f) });
    }
  }

  let hasImages = false;
  for (const task of assignment.tasks) {
    if (task.images.length > 0) {
      hasImages = true;
      break;
    }
  }

  if (hasImages) {
    blocks.push({ type: "bilag" });
  }


  for (const task of assignment.tasks) {
    if (task.images.length > 0) {
      blocks.push({ type: "bilag-title", id: task.id, text: task.title });
    }
    for (const img of task.images) {
      blocks.push({ type: "image", taskId: task.id, imageId: img.id, path: img.path });
    }
  }

  return blocks;
}

// Ooga booga
const blockKey = (block) => {
  switch (block.type) {
    case "title": return "title";
    case "task": return "task:" + block.id;
    case "explanation": return "explanation:" + block.formulaId;
    case "formula": return "formula:" + block.formulaId;
    case "bilag": return "bilag";
    case "bilag-title": return "bilag-title:" + block.id;
    case "image": return "image:" + block.imageId;
    default: return "?" + Math.random();
  }
}

// element creation
const katexRender = (el, latex) => {
  window.katex.render(latex, el, { throwOnError: false, displayMode: false });
}

const createElement = (block) => {
  switch (block.type) {
    case "title": {
      const el = document.createElement("h1");
      el.contentEditable = "true";
      el.innerText = block.text;
      return el;
    }
    case "task": {
      const el = document.createElement("h2");
      el.contentEditable = "true";
      el.innerText = block.text;
      el.id = "task:" + block.id;
      el.className = "task-title";
      return el;
    }
    case "explanation": {
      const el = document.createElement("p");
      el.innerText = block.text;
      return el;
    }
    case "formula": {
      const el = document.createElement("div");
      el.className = "formula";
      katexRender(el, block.latex);
      el._latex = block.latex; // Cache to not re render if same
      return el;
    }
    case "bilag": {
      const el = document.createElement("h1");
      el.innerText = "Bilag";
      return el;
    }
    case "bilag-title": {
      const el = document.createElement("h2");
      el.innerText = block.text;
      return el;
    }
    case "image": {
      const wrapper = document.createElement("div");
      wrapper.className = "image-block";
      const img = document.createElement("img");
      img.src = block.path;
      wrapper.appendChild(img);
      return wrapper;
    }
    default: return document.createElement("div");
  }
}

const attachListeners = (block, el) => {
  switch (block.type) {
    case "title": {
      el.addEventListener("input", debounce(
        () => bridge && bridge.updateTitle(el.innerText)
      ));
      break;
    }
    case "task": {
      el.addEventListener("input", debounce(
        () => bridge && bridge.updateTaskTitle(block.id, el.innerText)
      ));
      break;
    }
    case "image": {
      el.addEventListener(
        "click",
        () => bridge && bridge.removeImage(block.taskId, block.imageId)
      );
      break;
    }
  }
}

// Pagination
const PAGE_HEIGHT = 297 * 3.78 - 100;

const waitForImages = async (el) => {
  const imgs = el.querySelectorAll("img");
  await Promise.all([...imgs].map(async (img) => {
    if (!img.src) return;
    if (!img.complete) await new Promise(r => { img.onload = r; img.onerror = r; });
    if (img.decode) { try { await img.decode(); } catch(_) {} }
  }));
}

const measureHeight = (el) => {
  probe.innerHTML = "";
  // const clone = el.cloneNode(true);
  probe.appendChild(el);

  const rect = el.getBoundingClientRect();
  const style = window.getComputedStyle(el);

  const margin =
    rect.height +
    parseFloat(style.marginTop) +
    parseFloat(style.marginBottom);

  // const rectHeight = clone.getBoundingClientRect().height;

  // console.log(rectHeight + margin);
  return margin;
}

// const splitEquation = (expr) => {
//   const parts = expr.split("=");
//   return {
//     lhs: parts[0]?.trim(),
//     rhs: parts[1]?.trim()
//   };
// }
//
// const isNumericExpression = (expr) => {
//   return /^[\d.\s]+$/.test(expr);
// }
//
// const hasVariables = (expr) => {
//   return /[a-zA-Z]/.test(expr);
// }

// const getPageHeight = () => {
//   const page = createPage();
//   document.body.appendChild(page.page);
//   const height = page.content.getBoundingClientRect().height;
//   document.body.removeChild(page.page);
//   return height;
// }

const createPage = (assignment) => {
  const page = document.createElement("div");
  page.className = "page";

  const header = document.createElement("div");
  header.className = "page-header";

  const name = document.createElement("div");
  name.innerText = `Navn: ${
    assignment.names.length == 0 ? "Please do: Edit -> Names..."
    : assignment.names.length == 1 ? assignment.names[0] : ""
  }`;

  if (assignment.names.length > 1) {
    let str = "";
    for (let i = 0; i < assignment.names.length; i++) {
      str += assignment.names[i];
      if (i + 2 < assignment.names.length) {
        str += ", ";
      }

      if (i + 2 == assignment.names.length) {
        str += " og ";
      }
    }

    name.innerText = `Navne: ${str}`;
  }

  const date = document.createElement("div");
  date.innerText = `Dato: ${new Date().toLocaleDateString()}`;

  header.appendChild(name);
  header.appendChild(date);

  const content = document.createElement("div");
  content.className = "page-content";

  page.appendChild(header);
  page.appendChild(content);

  return { page, content };
}

const fullRender = async (blocks, assignment) => {
  blockRefs.clear();

  const pages = [];
  let cur = createPage(assignment);
  pages.push(cur);
  let heightUsed = 0;

  for (const block of blocks) {
    const el = createElement(block);
    const key = blockKey(block);

    const clone = el.cloneNode(true);
    await waitForImages(clone);
    const h = measureHeight(clone);

    if (heightUsed + h > PAGE_HEIGHT && heightUsed > 0) {
      cur = createPage(assignment);
      pages.push(cur);
      heightUsed = 0;
    }

    attachListeners(block, el);
    cur.content.appendChild(el);
    blockRefs.set(key, el);
    heightUsed += h;
  }

  pagesContainer.innerHTML = "";
  for (const { page } of pages) {
    pagesContainer.appendChild(page);
  }
}

const patchBlocks = (newBlocks) => {
  for (const block of newBlocks) {
    const el = blockRefs.get(blockKey(block));
    if (!el) continue;

    switch (block.type) {
      case "title":
      case "task":
      case "explanation":
      case "bilg-title":
        if (document.activeElement !== el && el.innerText != block.text) {
          el.innerText = block.text;
        }
        break;

      case "formula":
        if (el._latex !== block.latex) {
          el._latex = block.latex;
          katexRender(el, block.latex);
        }
        break;
    }
  }
}

// Smart update (smart, clever, very demure)
let rendering = false;
let queued = null;

const smartUpdate = async (assignment, force = false) => {
  if (rendering) {
    queued = assignment;
    return;
  }
  
  const blocks = buildBlocks(assignment);
  const newKeys = blocks.map(blockKey).join(",");

  if (force || !prevKeys || prevKeys !== newKeys) {
    rendering = true;
    lastScrollY = window.scrollY;
    await fullRender(blocks, assignment);
    prevKeys = newKeys;
    rendering = false;
    requestAnimationFrame(() => requestAnimationFrame(() => window.scrollTo(0, lastScrollY)));
  } else {
    patchBlocks(blocks);
    prevKeys = newKeys;
  }

  if (queued) {
    const next = queued;
    queued = null;
    smartUpdate(next);
  }
}

new QWebChannel(qt.webChannelTransport, function(channel) {
  bridge = channel.objects.bridge;

  window.bridge = bridge;

  bridge.setBgCol.connect(function(str) {
    document.body.style.background = str;
  });

  bridge.updatePages.connect(function(assignment) {
    lastScrollY = window.scrollY;
    // console.log("Last scroll pos: " + lastScrollPos);
    smartUpdate(JSON.parse(assignment));
  })

  bridge.updatePagesFull.connect(function(assignment) {
    lastScrollY = window.scrollY;
    // Haha no longer smart. Stupid
    smartUpdate(JSON.parse(assignment), true);
  })
  
  bridge.scrollToTask.connect(function(task) {
    scrollToTask(JSON.parse(task));
  })

  bridge.jsReady();
});

// const createText = (tag, text) => {
//   const el = document.createElement(tag);
//   el.innerText = text;
//   return el;
// }
//
// const buildBlocks = (assignment) => {
//   const blocks = [];
//
//   blocks.push({
//     type: "title",
//     el: createText("h1", assignment.title)
//   });
//
//   assignment.tasks.forEach(task => {
//     blocks.push({
//       type: "task",
//       id: task.id,
//       el: createText("h2", task.title)
//     });
//
//     task.formulas.forEach(f => {
//       if (f.explanation != "" && f.explanation != "\n") {
//         blocks.push({
//           type: "explanation",
//           el: createText("p", f.explanation)
//         })
//       }
//
//       const div = document.createElement("div");
//       div.className = "formula";
//
//       let latex = f.latex;
//       console.log("Latex: " + f.latex);
//       if (f.result != null && f.result != "") {
//         const { lhs, rhs } = splitEquation(f.latex);
//         if (rhs && hasVariables(rhs)) {
//           if (f.isAnswer) {
//             latex += "=\\underline{\\underline{" + f.result + "}}";
//           } else {
//             latex += "=" + f.result;
//           }
//         } else if (!rhs) {
//           if (f.isAnswer) {
//             latex += "=\\underline{\\underline{" + f.result + "}}";
//           } else {
//             latex += "=" + f.result;
//           }
//         }
//       } else {
//         if (f.isAnswer) {
//           latex = "\\underline{\\underline{" + latex + "}}";
//         }
//       }
//
//       katex.render(latex, div, { throwOnError: false });
//
//       blocks.push({
//         type: "formula",
//         el: div
//       });
//     });
//   });
//
//   blocks.push({
//     type: "bilag",
//     el: createText("h1", "Bilag"),
//   });
//
//   assignment.tasks.forEach(task => {
//     if (task.images.length > 0) {
//       blocks.push({
//         type: "bilag-title",
//         el: createText("h2", task.title),
//       });
//     }
//
//     task.images?.forEach(img => {
//       const wrapper = document.createElement("div");
//       wrapper.className = "image-block";
//
//       const image = document.createElement("img");
//       image.src = `${img.path}`;
//
//       wrapper.appendChild(image);
//
//       blocks.push({
//         type: "image",
//         taskId: task.id,
//         imageId: img.id,
//         el: wrapper
//       });
//     });
//   });
//
//   return blocks;
// }
//
// const waitForImages = async (el) => {
//   const imgs = el.querySelectorAll("img");
//
//   await Promise.all([...imgs].map(async (img) => {
//     if (!img.src) return;
//
//     if (!img.complete) {
//       await new Promise(res => {
//         img.onload = res;
//         img.onerror = res;
//       });
//     }
//
//     // important: forces real decoding (fixes Qt/WebEngine bugs)
//     if (img.decode) {
//       try { await img.decode(); } catch (e) {}
//     }
//   }));
// };
//
// const paginate = async (blocks) => {
//   const pages = [];
//
//   let page = createPage();
//   let height = 0;
//
//   const PAGE_HEIGHT = (297 * 3.78) - 100;
//
//   pages.push(page);
//
//   for (const block of blocks) {
//
//     if (block.type == "title" || block.type == "task") {
//       block.el.contentEditable = "true";
//     }
//     const clone = block.el.cloneNode(true);
//
//     await waitForImages(clone);
//
//     const h = measureHeight(clone);
//
//     console.log(h + height);
//     console.log(block.type + ": " + block.el.innerText + ": " + h);
//
//     if (height + h > PAGE_HEIGHT) {
//       page = createPage();
//       pages.push(page);
//       height = 0;
//     }
//
//     // const clone = block.el.cloneNode(true);
//
//     if (block.type == "title") {
//       clone.addEventListener("input", () => {
//         console.log("WOEOOEOEE");
//         bridge.updateTitle(clone.innerText);
//       })
//     } else if (block.type == "task") {
//       clone.addEventListener("input", () => {
//         bridge.updateTaskTitle(block.id, clone.innerText)
//       })
//     } else if (block.type == "image") {
//       clone.addEventListener("click", () => {
//         console.log("WRAPPER CLICKED");
//         bridge.removeImage(block.taskId, block.imageId);
//       })
//     }
//
//     page.content.appendChild(clone);
//     height += h;
//   };
//
//   return pages;
// }
//
// const renderLatex = (el, latex) => {
//   window.katex.render(latex, el, {
//     throwOnError: false
//   });
// };
//
// const renderPages = async (assignment) => {
//   pagesContainer.innerHTML = "";
//
//   const blocks = buildBlocks(assignment);
//   const pages = await paginate(blocks);
//
//   pages.forEach(p => {
//     pagesContainer.appendChild(p.page);
//   });
//
//   requestAnimationFrame(() => {
//     requestAnimationFrame(() => {
//       window.scrollTo(0, lastScrollPos);
//     })
//   })
// }

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
