"use strict";

const MQ = MathQuill.getInterface(2);

const debounce = (fn, delay = 400) => {
  let timer;
  return (...args) => {
    clearTimeout(timer);
    timer = setTimeout(() => fn(...args), delay);
  };
};

// === Simplified Row ===
class FormulaRow {
  constructor(id, initialData, hooks) {
    this.id = id;
    this.hooks = hooks;
    this.element = document.createElement('div');
    this.element.className = 'formula-row';

    const mqSpan = document.createElement('span');
    mqSpan.className = 'input-field';

    const removeBtn = document.createElement('button');
    removeBtn.className = 'remove-btn';
    removeBtn.textContent = 'X';
    removeBtn.onclick = () => this.hooks.onRemove(this.id);

    this.toggleBtn = document.createElement("button");
    this.toggleBtn.className = "btn";
    this.toggleBtn.textContent = 'Answer';
    this.toggleBtn.classList.toggle("active", initialData.isAnswer);
    this.toggleBtn.onclick = () => this.hooks.onToggleAnswer(this.id);

    this.element.append(mqSpan, this.toggleBtn, removeBtn);

    this.mf = MQ.MathField(mqSpan, {
      handlers: {
        edit: () => {
          if (this.isInternalChange) return;
          debounce(this.hooks.onUpdate(this.id, this.mf.latex()));
        },
        enter: () => this.hooks.onAddAfter(this.id)
      }
    });
    this.mf.latex(initialData.latex || '');
    this.mqArea = mqSpan.querySelector('textarea');
  }

  update(fData) {
    // Only update if the user isn't currently typing in this specific field
    if (document.activeElement !== this.mqArea && this.mf.latex() !== fData.latex) {
      this.isInternalChange = true;
      this.mf.latex(fData.latex);
      this.isInternalChange = false;
    }

    this.toggleBtn.classList.toggle("active", fData.isAnswer);
  }
}

class TaskRenderer {
  constructor(container, bridge) {
    this.container = container;
    this.bridge = bridge;
    this.rows = new Map(); // id -> FormulaRow
  }

  render(task) {
    console.log("RENDERING!!");
    if (!task) {
      this.container.innerHTML = '';
      this.rows.clear();
      return;
    }

    const newIds = task.formulas.map(f => f.id);

    // 1. Remove rows no longer in the task
    for (const [id, row] of this.rows.entries()) {
      if (!newIds.includes(id)) {
        row.element.remove();
        this.rows.delete(id);
      }
    }

    // 2. Add or Update rows
    task.formulas.forEach((fData, index) => {
      let row = this.rows.get(fData.id);
      if (!row) {
        row = new FormulaRow(fData.id, fData, {
          onUpdate: (id, latex) => this.bridge.updateFormula(id, latex),
          onRemove: (id) => this.bridge.removeFormula(id),
          onAddAfter: (id) => this.bridge.addFormulaAfter(id),
          onToggleAnswer: (id) => this.bridge.toggleAnswer(id)
        });
        this.rows.set(fData.id, row);
      } else {
        row.update(fData);
      }

      // 3. Ensure DOM order without re-inserting if already correct
      if (this.container.children[index] !== row.element) {
        this.container.insertBefore(row.element, this.container.children[index] || null);
      }
    });
  }

  focus(id) {
    this.rows.get(id)?.mf.focus();
  }

  blurAll() {
    if (document.activeElement) {
      document.activeElement.blur();
    }
  }
}

// --- Bootstrap ---
new QWebChannel(qt.webChannelTransport, (channel) => {
  const bridge = channel.objects.bridge;
  const renderer = new TaskRenderer(document.getElementById('formulas'), bridge);

  bridge.taskChanged.connect((json) => renderer.render(JSON.parse(json)));
  bridge.focusFormula.connect((id) => renderer.focus(id));

  bridge.lostFocus.connect(() => renderer.blurAll());

  // --- KEYBOARD REDIRECT ---
  window.addEventListener('keydown', (e) => {
    const isZ = e.key.toLowerCase() === 'z';
    const isY = e.key.toLowerCase() === 'y';
    
    if ((e.ctrlKey || e.metaKey) && isZ) {
      e.preventDefault(); // Stop browser undo
      renderer.blurAll();
      if (e.shiftKey) bridge.redo();
      else bridge.undo();
    } 
    else if ((e.ctrlKey || e.metaKey) && isY) {
      e.preventDefault();
      renderer.blurAll();
      bridge.redo();
    }
  }, true);
  
  document.getElementById('add-btn')?.addEventListener('click', () => bridge.addFormula());
});
