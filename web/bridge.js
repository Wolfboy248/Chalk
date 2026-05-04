"use strict";

// ============================================================
// LaTeX → Math.js  (pure string conversion, no side-effects)
// ============================================================

const latexToMathjs = (latex) => {
  let expr = latex
    .replace(/\\ /g,   '')
    .replace(/\\right\)/g, ')').replace(/\\left\(/g,  '(')
    .replace(/\\right\]/g, ']').replace(/\\left\[/g,  '[')
    .replace(/\\arcsin/g, 'asin').replace(/\\sin/g,   'sin')
    .replace(/\\arccos/g, 'acos').replace(/\\cos/g,   'cos')
    .replace(/\\arctan/g, 'atan').replace(/\\tan/g,   'tan')
    .replace(/\\Omega/g, 'ohm')
    .replace(/\\operatorname\{cross\}/g, 'cross')
    .replace(/\\pi/g,  'pi')
    .replace(/\\rho/g, 'rho')
    .replace(/\\int/g, 'int')
    .replace(/\\degree/g, ' * deg')
    .replace(/_\{\s*([a-zA-Z]+)\}/g,        '_$1')
    .replace(/\\vec\{([^}]+)\}/g,            '$1_vec')
    .replace(/\\angle\s*([a-zA-Z]+)/g,       '$1_angle')
    .replace(/\\Delta\s*([a-zA-Z]+)/g,       '$1_Delta')
    .replace(/\\sqrt\{([^}]+)\}/g,           'sqrt($1)')
    .replace(/\\frac\{([^}]+)\}\{([^}]+)\}/g,'($1)/($2)')
    .replace(/([0-9])\s*(ml|l|L|mol|cm|m|kg|V)/g, '$1 $2')
    .replace(/(\d),(?=\d)/g, '$1.')
    .replace(/;/g,     ',')
    .replace(/\\cdot/g,'*');

  // ±  →  [a+b, a-b]
  const pmParts = expr.split('\\pm');
  if (pmParts.length === 2) {
    const [l, r] = pmParts.map(s => s.trim());
    expr = `[(${l}) + (${r}), (${l}) - (${r})]`;
  }

  return expr.trim();
};

// ============================================================
// Evaluator  – pure function, no DOM, no bridge calls
// Returns: Array<{ id, result: string|null, error: string|null }>
// ============================================================

const evaluateFormulas = (() => {
  const MAX_PASSES = 10;

  const getAssignment = (expr) =>
    expr.match(/^([a-zA-Z][a-zA-Z0-9]*)\s*=/)?.[1] ?? null;

  const isRedefined = (varName, index, formulas) =>
    formulas.slice(0, index).some(f => getAssignment(latexToMathjs(f.latex)) === varName);

  const applyUnitOverride = (val, override) => {
    if (!override?.trim() || !val?.clone) return val;
    try { return val.clone().to(override); } catch { return val; }
  };

  const formatVal = (val) =>
    math.format(val, { precision: 4, notation: 'auto' })
      .replace(/,/g,   ';')
      .replace(/\./g,  ',\\!')
      .replace(/(\d)\s+(?!deg\b)([a-zA-Z]+)/g, '$1\\, $2')
      .replace(/ohm/g, '\\Omega')
      .replace(/ ?deg/g, '\\degree');

  const scopeSnapshot = (scope) =>
    Object.keys(scope).map(k => `${k}:${math.format(scope[k])}`).join('|');

  return (formulas) => {
    const results = formulas.map(f => ({ id: f.id, result: null, error: null }));
    const scope   = {};
    let prevSnap  = null;

    for (let pass = 0; pass < MAX_PASSES; pass++) {
      const snap = scopeSnapshot(scope);
      if (snap === prevSnap) break;
      prevSnap = snap;

      formulas.forEach((f, i) => {
        if (f.isIntermediate) return;
        try {
          const expr = latexToMathjs(f.latex);
          if (!expr) return;

          const assignedVar = getAssignment(expr);
          if (assignedVar && isRedefined(assignedVar, i, formulas)) {
            results[i].error = `${assignedVar} already defined`;
            return;
          }

          let val = math.evaluate(expr, scope);

          // Inverse trig always comes back in radians — promote to a unit
          if (typeof val === 'number' && /\b(asin|acos|atan)\b/.test(expr)) {
            val = math.unit(val, 'rad');
          }

          if (assignedVar) scope[assignedVar] = val?.clone ? val.clone() : val;

          results[i].result = formatVal(applyUnitOverride(val, f.unitOverride));
        } catch (e) {
          results[i].error = e.message;
        }
      });
    }
    return results;
  };
})();

// ============================================================
// Debounce  – with .flush() to fire immediately on blur
// ============================================================

const debounce = (fn, delay = 300) => {
  let timer;
  const d    = (...args) => { clearTimeout(timer); timer = setTimeout(() => fn(...args), delay); };
  d.flush    = (...args) => { clearTimeout(timer); fn(...args); };
  d.cancel   = ()        => clearTimeout(timer);
  return d;
};

// ============================================================
// FormulaRow  – owns the DOM subtree for one formula entry
//
// Hooks (all provided by TaskRenderer):
//   onSave(id, latex)            – persist latex to C++ (every keystroke)
//   onEvaluate()                 – trigger re-evaluation (debounced)
//   onExplanationChange(id, val) – persist explanation
//   onUnitOverrideChange(id,val) – persist + re-evaluate
//   onToggle(kind, id)           – 'answer' | 'hideAnswer' | 'intermediate'
//   onRemove(id)
//   onAddAfter(id)
//   onNavigateUp(id)
//   onNavigateDown(id)
// ============================================================

class FormulaRow {
  constructor(formula, hooks) {
    this.id       = formula.id;
    this._hooks   = hooks;
    this._mf      = null;   // MathQuill field
    this._mqArea  = null;   // <textarea> inside MQ span (for focus checks)
    this._resultEl   = null;
    this._unitEl     = null;
    this._exprEl     = null;
    this._showExpr   = false;
    this.element     = this._build(formula);
  }

  // ---- Public API called by TaskRenderer ----

  /** Sync DOM from new server-side formula data, skipping focused fields. */
  update(formula) {
    if (this._mqArea !== document.activeElement) {
      if (this._mf.latex() !== (formula.latex ?? ''))
        this._mf.latex(formula.latex ?? '');
    }
    if (this._exprEl !== document.activeElement)
      this._exprEl.value = formula.explanation ?? '';
    if (this._unitEl !== document.activeElement)
      this._unitEl.value = formula.unitOverride ?? '';
  }

  /** Render an evaluated result (or clear it). */
  showResult(result, error) {
    if (error) {
      this._resultEl.className = 'result error';
      this._resultEl.textContent = '';
    } else if (result) {
      this._resultEl.className = 'result';
      this._unitEl.className   = 'unit-override';
      window.katex.render('=' + result, this._resultEl, { throwOnError: false });
    } else {
      this._resultEl.className = 'result hidden';
      this._unitEl.className   = 'unit-override hidden';
    }
  }

  focus() { this._mf?.focus(); }

  // ---- Private ----

  _build(formula) {
    const h   = this._hooks;
    const id  = formula.id;

    // Debounced evaluation trigger — shared for edit + unit override
    const triggerEval = debounce(() => h.onEvaluate(), 250);

    // ---- Root ----
    const row = document.createElement('div');
    row.className = 'formula-row';

    // ---- MathQuill + remove ----
    const upper = document.createElement('div');
    upper.className = 'upper-container';

    const mqSpan = document.createElement('span');
    mqSpan.className = 'input-field';

    const removeBtn = document.createElement('button');
    removeBtn.className = 'remove-btn';
    removeBtn.textContent = '✕';
    removeBtn.addEventListener('click', () => h.onRemove(id));

    upper.append(mqSpan, removeBtn);

    // ---- Result + unit override ----
    const lower = document.createElement('div');
    lower.className = 'lower-container';

    this._resultEl = document.createElement('span');
    this._resultEl.className = 'result hidden';

    this._unitEl = document.createElement('input');
    this._unitEl.className   = 'unit-override hidden';
    this._unitEl.placeholder = 'unit';
    this._unitEl.value       = formula.unitOverride ?? '';
    this._unitEl.addEventListener('input', () => {
      h.onUnitOverrideChange(id, this._unitEl.value);
      triggerEval();
    });

    lower.append(this._resultEl, this._unitEl);

    const formulaContainer = document.createElement('div');
    formulaContainer.className = 'formula-container';
    formulaContainer.append(upper, lower);
    row.appendChild(formulaContainer);

    // ---- Toggle buttons ----
    const btnRow = document.createElement('div');
    btnRow.className = 'buttons-container';

    // Explanation toggle is local UI only
    const exprBtn = this._makeToggleBtn('Explanation', formula, false, () => {
      this._showExpr = !this._showExpr;
      exprBtn.classList.toggle('active', this._showExpr);
      this._exprEl.classList.toggle('shown', this._showExpr);
    });

    const answerBtn       = this._makeToggleBtn('Is Answer',    formula, formula.isAnswer,
      () => h.onToggle('answer', id));
    const hideAnswerBtn   = this._makeToggleBtn('Hide Answer',  formula, formula.hideAnswer,
      () => h.onToggle('hideAnswer', id));
    const intermediateBtn = this._makeToggleBtn('Intermediate', formula, formula.isIntermediate,
      () => h.onToggle('intermediate', id));

    btnRow.append(exprBtn, answerBtn, hideAnswerBtn, intermediateBtn);
    row.appendChild(btnRow);

    // ---- Explanation ----
    this._exprEl = document.createElement('input');
    this._exprEl.className   = 'explanation';
    this._exprEl.placeholder = 'Type explanation here...';
    this._exprEl.value       = formula.explanation ?? '';

    const debouncedExpl = debounce((v) => h.onExplanationChange(id, v), 400);
    this._exprEl.addEventListener('input', () => debouncedExpl(this._exprEl.value));
    this._exprEl.addEventListener('blur',  () => debouncedExpl.flush(this._exprEl.value));
    row.appendChild(this._exprEl);

    // ---- MathQuill ----
    this._mf = MQ.MathField(mqSpan, {
      autoCommands:              'pi theta sqrt sum angle degree Updownarrow underline vec delta Delta omega Omega pm',
      charsThatBreakOutOfSupSub: '+-=<>',
      autoSubscriptNumerals:     true,
      restrictMismatchedBrackets: false,
      autoOperatorNames:         'sin cos tan asin acos atan arcsin arccos arctan cross',
      handlers: {
        // Save every keystroke; evaluation is debounced separately
        edit:      () => { h.onSave(id, this._mf.latex()); triggerEval(); },
        enter:     () => h.onAddAfter(id),
        downOutOf: () => h.onNavigateDown(id),
        upOutOf:   () => h.onNavigateUp(id),
      }
    });

    this._mqArea = mqSpan.querySelector('textarea');
    this._mqArea.addEventListener('keydown', (e) => this._onKeyDown(e, id));
    this._mf.latex(formula.latex ?? '');

    return row;
  }

  _makeToggleBtn(label, formula, initialActive, onClick) {
    const btn = document.createElement('button');
    btn.className = 'btn' + (initialActive ? ' active' : '');
    btn.innerText = label;
    btn.addEventListener('click', () => { onClick(); btn.classList.toggle('active'); });
    return btn;
  }

  _onKeyDown(e, id) {
    if (e.ctrlKey) {
      const write = { '(': '(', ')': ')', '[': '[', ']': ']' }[e.key];
      if (write) { e.preventDefault(); this._mf.write(write); return; }
    }
    if (e.key === 'Backspace' && this._mf.latex() === '') {
      e.preventDefault();
      this._hooks.onRemove(id);
    }
  }
}

// ============================================================
// TaskRenderer  – manages the ordered set of FormulaRows
//
// All communication with C++ goes through `bridge`.
// Navigation and evaluation are handled locally.
// ============================================================

class TaskRenderer {
  constructor(container, bridge) {
    this._container = container;
    this._bridge    = bridge;
    this._rows      = new Map();  // id → FormulaRow
    this._order     = [];         // ordered list of ids
    this._task      = null;       // last received task (kept in sync for eval)
  }

  /** Full diff-and-patch from a new task object received from C++. */
  render(task) {
    console.log("CALLED RENDER!!!");
    this._task = task;

    if (!task) {
      this._clear();
      return;
    }

    const newIds = task.formulas.map(f => f.id);

    // Remove rows that no longer exist
    for (const id of this._order) {
      if (!newIds.includes(id)) {
        this._rows.get(id)?.element.remove();
        this._rows.delete(id);
      }
    }

    // Create new rows / patch existing ones
    for (const formula of task.formulas) {
      if (this._rows.has(formula.id)) {
        this._rows.get(formula.id).update(formula);
      } else {
        this._rows.set(formula.id, new FormulaRow(formula, this._makeHooks()));
      }
    }

    // Ensure DOM order matches server order (appendChild moves existing nodes)
    for (const formula of task.formulas) {
      this._container.appendChild(this._rows.get(formula.id).element);
    }

    this._order = newIds;
    this._evaluate(); // initial evaluation when task loads
  }

  focus(id) { this._rows.get(id)?.focus(); }

  applyResults(results) {
    for (const r of results) this._rows.get(r.id)?.showResult(r.result, r.error);
  }

  // ---- Private ----

  _evaluate() {
    if (!this._task) return;
    const results = evaluateFormulas(this._task.formulas);
    this.applyResults(results);
    // Send back to C++ so results persist in the Assignment struct
    this._bridge.receiveResults(JSON.stringify(results));
  }

  _navigateDown(fromId) {
    const idx = this._order.indexOf(fromId);
    if (idx < this._order.length - 1) {
      this.focus(this._order[idx + 1]);
    } else {
      // At the last row: ask C++ to create a new formula, focus follows via focusFormula signal
      this._bridge.addFormulaAfter(fromId);
    }
  }

  _navigateUp(fromId) {
    const idx = this._order.indexOf(fromId);
    if (idx > 0) this.focus(this._order[idx - 1]);
  }

  _clear() {
    this._container.innerHTML = '';
    this._rows.clear();
    this._order = [];
    this._task  = null;
  }

  /** Hook object passed to each FormulaRow. Closures capture `this` safely. */
  _makeHooks() {
    const bridge = this._bridge;
    return {
      onSave: (id, latex) => {
        console.error("onSave");
        bridge.updateFormula(id, latex);
        // Keep local task in sync so _evaluate() always sees current latex
        const f = this._task?.formulas.find(f => f.id === id);
        if (f) f.latex = latex;
      },

      onEvaluate: () => this._evaluate(),

      onExplanationChange: (id, val) => {
        bridge.updateExplanation(id, val);
        const f = this._task?.formulas.find(f => f.id === id);
        if (f) f.explanation = val;
      },

      onUnitOverrideChange: (id, val) => {
        bridge.updateUnitoverride(id, val);
        const f = this._task?.formulas.find(f => f.id === id);
        if (f) f.unitOverride = val;
      },

      onToggle: (kind, id) => {
        const calls = {
          answer:       () => bridge.toggleAnswer(id),
          hideAnswer:   () => bridge.toggleHideAnswer(id),
          intermediate: () => bridge.toggleIntermediate(id),
        };
        calls[kind]?.();
        // C++ will emit taskChanged which re-renders and re-evaluates
      },

      onRemove:       (id)  => bridge.removeFormula(id),
      onAddAfter:     (id)  => bridge.addFormulaAfter(id),
      onNavigateDown: (id)  => this._navigateDown(id),
      onNavigateUp:   (id)  => this._navigateUp(id),
    };
  }
}

// ============================================================
// Bootstrap
// ============================================================

const MQ = MathQuill.getInterface(2);
let renderer = null;

new QWebChannel(qt.webChannelTransport, function(channel) {
  const bridge = channel.objects.bridge;
  window.bridge = bridge;

  renderer = new TaskRenderer(document.getElementById('formulas'), bridge);

  // C++ → JS: task structure changed (add/remove/reorder formulas, toggle flags)
  bridge.taskChanged.connect((json) => {
    renderer.render(JSON.parse(json));
  });

  bridge.refreshed.connect((json) => {
    renderer.update(JSON.parse(json));
  })

  // C++ → JS: focus a specific formula after it was created
  bridge.focusFormula.connect((id) => renderer.focus(id));

  // Add button in toolbar
  document.getElementById('add-btn')?.addEventListener('click', () => bridge.addFormula());
});
