var bridge = null;
var MQ = MathQuill.getInterface(2);
var mathFieldSpan = document.getElementById('math-field');
var fields = {};
var idToFocus = null;
var useDummyData = false;

var dummyData = {
  title: "Test",
  formulas: [
    {
      id: 0,
      latex: "a=2+3",
      explanation: "This is an explanation"
    },
    {
      id: 1,
      latex: "b=t^2",
      explanation: "This is an explanation"
    }
  ]
}

if (useDummyData) {
  renderTask(dummyData);
}

const cleanDegrees = (expr) => {
  return expr.replace(/\\degree/g, "");
}

var mathField = MQ.MathField(mathFieldSpan, {
  handlers: {
    edit: function() {
      var latex = mathField.latex();
      if (window.bridge) {
        window.bridge.receiveLatex(latex);
      }
    }
  }
})

function expandPlusMinus(expr) {
  const parts = expr.split("\\pm");
  if (parts.length !== 2) return expr;

  const left = parts[0].trim();
  const right = parts[1].trim();

  return `[(${left}) + (${right}), (${left}) - (${right})]`;
}

function latexToMathjs(latex) {
  console.log("OMGOMGOMGOGOGOGMG");
  console.log("Input: " + latex);
  console.log([...latex]);
  console.log(latex.length);

  let result = latex
    .replace(/\\ /g, '')
    .replace(/\\right\)/g, ')')
    .replace(/\\left\(/g, '(')
    .replace(/\\right\]/g, ']')
    .replace(/\\left\[/g, '[')
    .replace(/\\arcsin/g, "asin")
    .replace(/\\sin/g, "sin")
    .replace(/\\arccos/g, "acos")
    .replace(/\\cos/g, "cos")
    .replace(/\\arctan/g, "atan")
    .replace(/\\operatorname\{cross\}/g, "cross")
    .replace(/\\tan/g, "tan")
    .replace(/\\int/g, "int")
    .replace(/\\pi/g, "pi")
    .replace(/\\degree/g, " deg")
    .replace(/_\{\s*([a-zA-Z]+)\}/g, "_$1")
    // .replace(/\\degree/g, "")
    .replace(/\\frac\{([^}]+)\}\{([^}]+)\}/g, '($1)/($2)')
    .replace(/\\vec\{([^}]+)\}/g, '$1_vec')
    .replace(/\\angle\s*([a-zA-Z]+)/g, '$1_angle')
    .replace(/\\Delta\s*([a-zA-Z]+)/g, '$1_Delta')
    .replace(/\\sqrt\{([^}]+)\}/g, 'sqrt($1)')

    .replace(/([0-9])\s*(ml|l|L|mol|cm|m|kg|V)/g, "$1 $2")

    .replace(/(\d),(?=\d)/g, '$1.')
    .replace(/;/g, ',')

    .replace(/\\cdot/g, "*");

  result = expandPlusMinus(result);

  console.log("OMGOMGOGMOGOGMOG22222");
  console.log("Output: " + result);

  return result.trim();
}

const mathJsResultToLatex = (val) => {
  let res = val
    .replace(/ ?deg/g, "\\degree")
    // .replace(/\\degree![a-zA-Z], "$1")

  return res;
}

new QWebChannel(qt.webChannelTransport, function(channel) {
  bridge = channel.objects.bridge;
  window.bridge = channel.objects.bridge;

  bridge.focusFormula.connect(function(id) {
    idToFocus = id;
  });

  bridge.taskChanged.connect(function(json) {
    renderTask(JSON.parse(json));
  });

  bridge.resultsReady.connect(function(json) {
    // console.log(json);
    renderResults(JSON.parse(json).formulas);
  })

  // AI MADE THIS!!!
  bridge.evaluateTask.connect(function(json) {
    console.log("EVALUATING!!!");
    var task = JSON.parse(json);
    var MAX_PASSES = 10;
    var results = task.formulas.map(function(f) {
      return { id: f.id, result: null, error: null };
    });

    var scope = {};
    var prevScope = null;

    for (var pass = 0; pass < MAX_PASSES; pass++) {
      var scopeSnapshot = JSON.stringify(scope);
      if (scopeSnapshot === prevScope) break; // stabilised, no need to continue
      prevScope = scopeSnapshot;

      task.formulas.forEach(function(f, i) {
        try {
          if (f.isIntermediate) return;
          results[i].show = true;
          var expr = latexToMathjs(f.latex);
          // console.log(expr);
          if (expr.trim() === '') return;

          // check for redefinition
          var assignMatch = expr.match(/^([a-zA-Z][a-zA-Z0-9]*)\s*=/);
          if (assignMatch) {
            var varName = assignMatch[1];
            var alreadyDefinedBy = task.formulas.slice(0, i).find(function(prev) {
              var prevExpr = latexToMathjs(prev.latex);
              var prevMatch = prevExpr.match(/^([a-zA-Z][a-zA-Z0-9]*)\s*=/);
              return prevMatch && prevMatch[1] === varName;
            });
            if (alreadyDefinedBy) {
              results[i].error = varName + ' already defined';
              return;
            }
          }

          if (assignMatch) {
            const varName = assignMatch[1];

            // if result is plain number but came from trig context, treat as angle
            if (typeof val === "number" && /\b(asin|acos|atan)\b/.test(expr)) {
              scope[varName] = val; // already converted to unit above
            } else if (val && val.isUnit) {
              scope[varName] = val;
            } else {
              scope[varName] = val;
            }
          }

          var val = math.evaluate(expr, scope);
          assignMatch = expr.match(/^([a-zA-Z][a-zA-Z0-9]*)\s*=/);

          if (typeof val === "number") {
            if (/\b(asin|acos|atan)\b/.test(expr)) {
              val = math.unit(val, "rad").toNumber("deg");
              val = math.unit(val, "deg");

              if (assignMatch) {
                scope[assignMatch[1]] = val;
              }
            }
          }

          if (typeof val === "number") {
            if (/\b(asin|acos|atan)\b/.test(expr)) {
              val = math.unit(val, "rad").toNumber("deg");
              val = math.unit(val, "deg");

              // IMPORTANT: store converted value in scope
              if (assignMatch) {
                scope[assignMatch[1]] = val;
              }
            }
          }

          if (typeof val === "number") {
            if (/asin|acos|atan/.test(expr)) {
              val = math.unit(val, "rad").toNumber("deg");
              val = math.unit(val, "deg");
            }
          }

          if (val && val.isUnit) {
            const formatted = math.format(val, {
              precision: 4,
              notation: "auto"
            });

            const normalizedInput = expr.replace(/\s+/g, '');
            const normalizedOutput = formatted.replace(/\s+/g, '');

            if (normalizedInput.endsWith(normalizedOutput)) {
              results[i].result = "";
              return;
            }

            if (val && val.isUnit) {
              try {
                const original = math.evaluate(expr);
                if (math.equal(val, original)) {
                  results[i].result = "";
                  // results[i].error = "__DO_NOT_SHOW__";
                  return;
                }
              } catch {

              }
            }

            console.log(math.format(val, { precision: 4 }));
            results[i].result = mathJsResultToLatex(
              math.format(val, {
                precision: 4,
                notation: "auto",
              }).replace(/,/g, ";").replace(/\./g, ",\\!").replace(/(\d)\s+(?!deg\b)([a-zA-Z]+)/g, "$1\\, $2")
            );
          } else {
            const formatted = math.format(val, {
              precision: 4,
              notation: "auto"
            });

            const normalizedInput = expr.replace(/\s+/g, '');
            const normalizedOutput = formatted.replace(/\s+/g, '');

            if (normalizedInput.endsWith(normalizedOutput)) {
              results[i].result = "";
              return;
            }
            // val = math.round(val, 10);
            results[i].result = mathJsResultToLatex(
              math.format(val, { precision: 4 })
            ).replace(/,/g, ";")
              .replace(/\./g, ",\\!")
              .replace(/(\d)\s+(?!deg\b)([a-zA-Z]+)/g, "$1\\, $2");
            // results[i].result = math.format(val, { precision: 4 })
          }

          // results[i].result = math.format(val, { precision: 10 });
          // results[i].error = null;
        } catch(e) {
          // console.error(e.message);
          results[i].error = e.message;
          results[i].result = null;
        }
      });
    }

    bridge.receiveResults(JSON.stringify(results));
  });

  // bridge.evaluateTask.connect(function(json) {
  //   var task = JSON.parse(json);
  //   var scope = {};
  //   var results = task.formulas.map(function(f) {
  //     var result = null;
  //     try {
  //       var expr = latexToMathjs(f.latex);
  //       if (expr.trim() !== '') {
  //         var val = math.evaluate(expr, scope);
  //         result = math.format(val, { precision: 6 });
  //       }
  //     } catch (e) {
  //       console.error(f.latex + ": " + e);
  //     }
  //     return { id: f.id, result: result };
  //   })
  //
  //   bridge.receiveResults(JSON.stringify(results));
  // })

  document.getElementById("add-btn").addEventListener("click", function() {
    bridge.addFormula();
  });
})

function renderTask(task) {
  var container = document.getElementById('formulas');
  container.innerHTML = '';
  if (!task) return;
  console.log(task);
  fields = [];

  console.log(task.formulas);

  var index = 0;
  task.formulas.forEach(function(f) {
    console.log(f);
    var row = document.createElement("div");
    row.className = "formula-row";

    row._showExplanation = false;

    const formulaContainer = document.createElement("div");
    formulaContainer.className = "formula-container";
    row.appendChild(formulaContainer);

    const upperContainer = document.createElement("div");
    upperContainer.className = "upper-container";
    const lowerContainer = document.createElement("div");
    lowerContainer.className = "lower-container";

    formulaContainer.appendChild(upperContainer);
    formulaContainer.appendChild(lowerContainer);

    var mqSpan = document.createElement("span");
    mqSpan.className = "input-field";
    upperContainer.appendChild(mqSpan);

    // remove btn
    var removeBtn = document.createElement("button");
    removeBtn.className = "remove-btn"
    removeBtn.textContent = "X";
    removeBtn.addEventListener("click", function() {
      bridge.removeFormula(f.id);
    });
    upperContainer.appendChild(removeBtn);

    // Result/error
    var resultDiv = document.createElement("span");
    resultDiv.id = "result-" + f.id;
    resultDiv.className = "result hidden";
    if (f.result != null && f.result != "") {
      resultDiv.className = "result";
    }
    lowerContainer.appendChild(resultDiv);

    let buttonsContainer = document.createElement("div");
    buttonsContainer.className = "buttons-container";
    row.appendChild(buttonsContainer);

    let explanation = document.createElement("input");
    explanation.value = f.explanation;
    explanation.placeholder = "Type explanation here...";

    const explanationToggleButton = document.createElement("button");
    explanationToggleButton.innerText = "Explanation";
    explanationToggleButton.title = "Add explanation";
    explanationToggleButton.className = "btn";
    explanationToggleButton.addEventListener("click", () => {
      console.log(row._showExplanation);
      row._showExplanation = !row._showExplanation;
      if (row._showExplanation) {
        explanationToggleButton.className = "btn active";
        explanation.className = "explanation shown";
      } else {
        explanationToggleButton.className = "btn";
        explanation.className = "explanation";
      }
    })
    buttonsContainer.appendChild(explanationToggleButton);

    let answerToggleButton = document.createElement("button");
    answerToggleButton.innerText = "Is Answer";
    answerToggleButton.className = "btn";
    answerToggleButton._isAnswer = f.isAnswer;
    if (f.isAnswer) {
      answerToggleButton.className = "btn active";
    }
    answerToggleButton.addEventListener("click", () => {
      console.log("OMG!");
      answerToggleButton._isAnswer = !answerToggleButton._isAnswer;
      if (answerToggleButton._isAnswer) {
        answerToggleButton.className = "btn active";
      } else {
        answerToggleButton.className = "btn";
      }
      bridge.toggleAnswer(f.id);
    })

    buttonsContainer.appendChild(answerToggleButton);

    let intermediateToggleBtn = document.createElement("button");
    intermediateToggleBtn.innerText = "Is Intermediate";
    intermediateToggleBtn.className = "btn";
    intermediateToggleBtn._isIntermediate = f.isIntermediate;
    if (f.isIntermediate) {
      intermediateToggleBtn.className = "btn active";
    }
    intermediateToggleBtn.addEventListener("click", () => {
      console.log("OMG!");
      intermediateToggleBtn._isIntermediate = !intermediateToggleBtn._isIntermediate;
      if (intermediateToggleBtn._isIntermediate) {
        intermediateToggleBtn.className = "btn active";
      } else {
        intermediateToggleBtn.className = "btn";
      }
      bridge.toggleIntermediate(f.id);
    })

    buttonsContainer.appendChild(intermediateToggleBtn);

    explanation.className = "explanation";
    explanation.innerText = f.explanation;
    explanation.contentEditable = "true";

    explanation.addEventListener("input", () => {
      console.log(explanation.value);
      bridge.updateExplanation(f.id, explanation.value);
    })
    row.appendChild(explanation);

    container.appendChild(row);

    var mf = MQ.MathField(mqSpan, {
      autoCommands: 'pi theta sqrt sum angle degree Updownarrow underline vec delta Delta omega Omega pm',
      charsThatBreakOutOfSupSub: '+-=<>',
      autoSubscriptNumerals: true,
      autoOperatorNames: 'sin cos tan asin acos atan arcsin arccos arctan cross',
      handlers: {
        edit: function() {
          bridge.updateFormula(f.id, mf.latex());
        },

        enter: function() {
          bridge.addFormulaAfter(f.id);
          var ids = Object.keys(fields).map(Number);
          var nextId = ids[ids.indexOf(f.id) + 1];
          if (nextId !== undefined) {
            fields[nextId].focus();
          }
        },

        downOutOf: function() {
          var ids = Object.keys(fields).map(Number);
          var nextId = ids[ids.indexOf(f.id) + 1];
          if (nextId != undefined) {
            fields[nextId].focus();
          } else {
            bridge.addFormulaAfter(f.id);
            idToFocus = nextId;
          }
        },
        upOutOf: function() {
          console.log(fields);
          var nextId = fields.find(obj => obj.id === f.id);
          console.log(nextId);
          if (nextId != undefined) {
            nextId.focus();
          }
        },
      }
    });

    var textArea = mqSpan.querySelector("textarea");
    textArea.addEventListener("keydown", function(e) {
      if (e.key === "Backspace" && mf.latex() === '') {
        e.preventDefault();

        var ids = Object.keys(fields).map(Number);
        var prevId = ids[ids.indexOf(f.id) - 1];
        if (prevId !== undefined) {
          fields[prevId].focus();
        }

        bridge.removeFormula(f.id);
      }
    })

    mf.latex(f.latex);
    fields[f.id] = mf;
    console.log(mf);

    if (idToFocus == f.id) {
      mf.focus();
      idToFocus = null;
    }
    if (fields[idToFocus])
      fields[idToFocus].focus();
    index++;
  })
}

function renderResults(results) {
  results.forEach(function(r) {
    let span = document.getElementById("result-" + r.id);
    if (!span) return;
    if (r.error) {
      span.className = "result error";
      // span.textContent = r.error;
    } else if (r.result != "") {
      span.className = "result";
      window.katex.render("=" + r.result, span, { throwOnError: false });
      // span.textContent = "= " + r.result;
    } else {
      span.className = "result hidden";
      // span.textContent = "";
    }
  })
}

