let bridge;
const pagesContainer = document.getElementById("pages");

new QWebChannel(qt.webChannelTransport, function(channel) {
  bridge = channel.objects.bridge;

  window.bridge = bridge;

  bridge.setBgCol.connect(function(str) {
    document.body.style.background = str;
  });

  bridge.updatePages.connect(function(assignment) {
    renderPages(JSON.parse(assignment));
  })

  bridge.jsReady();
});

const createPage = () => {
  const page = document.createElement("div");
  page.className = "page";
  return page;
}

const renderPages = (assignment) => {
  console.log(assignment);
  pages.innerHTML = ``;

  let currentPage = createPage();
  pages.appendChild(currentPage);

  const addPageBreak = () => {
    currentPage = createPage();
    pages.appendChild(currentPage);
  }

  const addElement = (el) => {
    currentPage.appendChild(el);
    if (currentPage.scrollHeight > currentPage.clientHeight) {
      currentPage.removeChild(el);
      currentPage = createPage();
      pages.appendChild(currentPage);
      currentPage.appendChild(el);
    }
  }

  // Title
  const titleEl = document.createElement("h1");
  titleEl.innerText = assignment.title;
  addElement(titleEl);

  assignment.tasks.forEach(task => {
    addPageBreak();

    const el = document.createElement("div");
    el.className = "task page-break";
    el.innerText = task.title;
    addElement(el);
  });
}
