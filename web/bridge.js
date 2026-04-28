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
    .replace(/\\Omega/g, "ohm")
    .replace(/\\arctan/g, "atan")
    .replace(/\\operatorname\{cross\}/g, "cross")
    .replace(/\\tan/g, "tan")
    .replace(/\\int/g, "int")
    .replace(/\\pi/g, "pi")
    .replace(/\\rho/g, "rho")
    // .replace(/([0-9]+(?:\.\d+)?)\\degree/g, '($1 * pi / 180)')
    .replace(/\\degree/g, " * deg")
    .replace(/_\{\s*([a-zA-Z]+)\}/g, "_$1")
    // .replace(/\\degree/g, "")
    .replace(/\\vec\{([^}]+)\}/g, '$1_vec')
    .replace(/\\angle\s*([a-zA-Z]+)/g, '$1_angle')
    .replace(/\\Delta\s*([a-zA-Z]+)/g, '$1_Delta')
    .replace(/\\sqrt\{([^}]+)\}/g, 'sqrt($1)')
    .replace(/\\frac\{([^}]+)\}\{([^}]+)\}/g, '($1)/($2)')

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

function getRHS(expr) {
  const match = expr.match(/^[a-zA-Z][a-zA-Z0-9]*\s*=\s*(.+)$/);
  return match ? match[1] : null;
}

function evaluateFormula(f, i, task, scope, results) {
  try {
    if (f.isIntermediate) return;

    results[i].show = true;

    const expr = latexToMathjs(f.latex);
    if (!expr.trim()) return;

    const assignMatch = getAssignment(expr);

    if (assignMatch && isRedefined(assignMatch, i, task)) {
      results[i].error = assignMatch + " already defined";
      console.error(results[i].error);
      return;
    }

    let val = math.evaluate(expr, scope);

    val = normalizeTrig(val, expr);

    if (assignMatch) {
      scope[assignMatch] = cloneIfPossible(val);
    }

    const displayVal = applyUnitOverride(val, f.unitOverride);

    // if (shouldHideResult(expr, val)) {
    //   results[i].result = "";
    //   return;
    // }
    // if (assignMatch && isTrivialAssignment(expr, scope)) {
    //   results[i].result = "";
    //   return;
    // }

    // if (isTrivialAssignment(expr)) {
    //   results[i].result = "";
    //   return;
    // }

    results[i].result = formatResult(displayVal);

  } catch (e) {
    results[i].error = e.message;
    results[i].result = null;
  }
}

function hasRealComputation(rhs) {
  return /[\+\-\*\/\^]|sqrt|sin|cos|tan/.test(rhs);
}

function isTrivialAssignment(expr, scope) {
  const rhs = getRHS(expr);
  if (!rhs) return false;

  // strip whitespace
  const clean = rhs.replace(/\s+/g, '');

  // CASE 1: contains operators → NOT trivial
  if (/[\+\-\*\/\^()]/.test(clean)) {
    return false;
  }

  // CASE 2: contains functions → NOT trivial
  if (/sin|cos|tan|sqrt|log|ln/.test(clean)) {
    return false;
  }

  // CASE 3: contains variables → NOT trivial
  const vars = Object.keys(scope || {});
  if (vars.some(v => clean.includes(v))) {
    return false;
  }

  // CASE 4: pure number/unit → trivial
  return true;
}

// function isTrivialAssignment(expr, val, scope) {
//   const rhs = getRHS(expr);
//   if (!rhs) return false;
//
//   try {
//     const rhsVal = math.evaluate(rhs, scope);
//
//     if (val && val.isUnit && rhsVal && rhsVal.isUnit) {
//       return math.equal(val, rhsVal);
//     }
//
//     return val === rhsVal;
//   } catch {
//     return false;
//   }
// }

// function isTrivialAssignment(expr) {
//   const rhs = getRHS(expr);
//   if (!rhs) return false;
//
//   const assignMatch = expr.match(/^([a-zA-Z][a-zA-Z0-9]*)\s*=/);
//   if (!assignMatch) return false;
//
//   const varName = assignMatch[1];
//
//   try {
//     const rhsVal = math.evaluate(rhs, scope);
//     const fullVal = math.evaluate(expr, scope);
//
//     if (rhsVal && rhsVal.isUnit && fullVal && fullVal.isUnit) {
//       return math.equal(rhsVal, fullVal);
//     }
//
//     return rhsVal === fullVal;
//   } catch {
//     return false;
//   }
//   // const match = expr.match(/^([a-zA-Z][a-zA-Z0-9]*)\s*=\s*(.+)$/);
//   // if (!match) return false;
//   //
//   // const rhs = match[2];
//   // return !hasRealComputation(rhs);
// }

function getAssignment(expr) {
  const match = expr.match(/^([a-zA-Z][a-zA-Z0-9]*)\s*=/);
  return match ? match[1] : null;
}

function isRedefined(varName, index, task) {
  return task.formulas.slice(0, index).some(prev => {
    const prevExpr = latexToMathjs(prev.latex);
    return getAssignment(prevExpr) === varName;
  });
}

function cloneIfPossible(val) {
  if (val && val.isUnit) {
    return val.clone().to("deg")
  }
  return val
  return val && val.clone ? val.clone().to("deg") : val;
}

function normalizeTrig(val, expr) {
  if (typeof val === "number" && /\b(asin|acos|atan)\b/.test(expr)) {
    return val;
    // const deg = math.unit(val, "rad").toNumber("deg");
    // return math.unit(deg, "deg");
  }
  return val;
}

function applyUnitOverride(val, unitOverride) {
  if (!unitOverride || !unitOverride.trim()) return val;

  // if (unitOverride === "%") {
  //   if (typeof val === "number") {
  //     return val * 100;
  //   }
  //   if (val && val.isUnit) {
  //     try {
  //       const num = val.toNumber();
  //       return num * 100;
  //     } catch {
  //       return val;
  //     }
  //   }
  // }

  try {
    return val && val.clone ? val.clone().to(unitOverride) : val;
  } catch {
    return val;
  }
}

function shouldHideResult(expr, val) {
  const formatted = math.format(val, { precision: 4 });
  return expr.replace(/\s+/g, '') === formatted.replace(/\s+/g, '');
}

function formatLatex(lat) {
  let str = lat;

  str = str
    .replace(/\,/g, ",\\!")
}

function formatResult(val) {
  // if (typeof val === "number" && isAngleExpression) {
  //   const deg = val * 180 / Math.PI
  //   return deg + "\\degree"
  // }
  let str = math.format(val, {
    precision: 4,
    notation: "auto"
  });

  str = str
    .replace(/,/g, ";")
    .replace(/\./g, ",\\!")
    .replace(/(\d)\s+(?!deg\b)([a-zA-Z]+)/g, "$1\\, $2")
    .replace(/ohm/g, "\\Omega");

  return mathJsResultToLatex(str);
}

function snapshotScope(scope) {
  return Object.keys(scope)
    .map(k => k + ":" + math.format(scope[k]))
    .join("|");
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
  const MAX_PASSES = 10;
  bridge.evaluateTask.connect(function(json) {
    const task = JSON.parse(json);

    const results = task.formulas.map(f => ({
      id: f.id,
      result: null,
      error: null,
      show: false
    }));

    let scope = {};
    let prevSnapshot = null;

    for (let pass = 0; pass < MAX_PASSES; pass++) {
      const snapshot = snapshotScope(scope);
      if (snapshot === prevSnapshot) break;
      prevSnapshot = snapshot;

      task.formulas.forEach((f, i) => {
        evaluateFormula(f, i, task, scope, results);
      });
    }

    bridge.receiveResults(JSON.stringify(results));
  });
  // bridge.evaluateTask.connect(function(json) {
  //   console.log("EVALUATING!!!");
  //   var task = JSON.parse(json);
  //   var MAX_PASSES = 10;
  //   var results = task.formulas.map(function(f) {
  //     return { id: f.id, result: null, error: null };
  //   });
  //
  //   var scope = {};
  //   var prevScope = null;
  //
  //   for (var pass = 0; pass < MAX_PASSES; pass++) {
  //     var scopeSnapshot = JSON.stringify(scope);
  //     if (scopeSnapshot === prevScope) break; // stabilised, no need to continue
  //     prevScope = scopeSnapshot;
  //
  //     task.formulas.forEach(function(f, i) {
  //       try {
  //         if (f.isIntermediate) return;
  //         results[i].show = true;
  //         var expr = latexToMathjs(f.latex);
  //         // console.log(expr);
  //         if (expr.trim() === '') return;
  //
  //         // check for redefinition
  //         var assignMatch = expr.match(/^([a-zA-Z][a-zA-Z0-9]*)\s*=/);
  //         if (assignMatch) {
  //           var varName = assignMatch[1];
  //           var alreadyDefinedBy = task.formulas.slice(0, i).find(function(prev) {
  //             var prevExpr = latexToMathjs(prev.latex);
  //             var prevMatch = prevExpr.match(/^([a-zA-Z][a-zA-Z0-9]*)\s*=/);
  //             return prevMatch && prevMatch[1] === varName;
  //           });
  //           if (alreadyDefinedBy) {
  //             results[i].error = varName + ' already defined';
  //             return;
  //           }
  //         }
  //
  //         var val = math.evaluate(expr, scope);
  //         assignMatch = expr.match(/^([a-zA-Z][a-zA-Z0-9]*)\s*=/);
  //
  //         if (assignMatch) {
  //           scope[assignMatch[1]] = val && val.clone ? val.clone() : val;
  //         }
  //
  //         if (typeof val === "number" && /\b(asin|acos|atan)\b/.test(expr)) {
  //           val = math.unit(val, "rad").toNumber("deg");
  //           val = math.unit(val, "deg");
  //
  //           if (assignMatch) {
  //             scope[assignMatch[1]] = val;
  //           }
  //         }
  //
  //         if (val && val.isUnit) {
  //           const formatted = math.format(val, {
  //             precision: 4,
  //             notation: "auto"
  //           });
  //
  //           const normalizedInput = expr.replace(/\s+/g, '');
  //           const normalizedOutput = formatted.replace(/\s+/g, '');
  //
  //           if (normalizedInput.endsWith(normalizedOutput)) {
  //             results[i].result = "";
  //             return;
  //           }
  //
  //           let displayVal = val;
  //
  //           if (f.unitOverride && f.unitOverride.trim() !== "") {
  //             try {
  //               displayVal = val.clone().to(f.unitOverride);
  //             } catch (e) {
  //
  //             }
  //           }
  //
  //           // if (val && val.isUnit) {
  //           //   try {
  //           //     // const original = math.evaluate(expr);
  //           //     if (math.equal(displayVal, original)) {
  //           //       results[i].result = "";
  //           //       // results[i].error = "__DO_NOT_SHOW__";
  //           //       return;
  //           //     }
  //           //   } catch {
  //           //
  //           //   }
  //           // }
  //
  //           console.log(math.format(val, { precision: 4 }));
  //           results[i].result = mathJsResultToLatex(
  //             math.format(displayVal, {
  //               precision: 4,
  //               notation: "exponential",
  //             }).replace(/,/g, ";").replace(/\./g, ",\\!").replace(/(\d)\s+(?!deg\b)([a-zA-Z]+)/g, "$1\\, $2")
  //           );
  //         } else {
  //
  //           let displayVal = val;
  //
  //           if (f.unitOverride && f.unitOverride.trim() !== "") {
  //             try {
  //               displayVal = val.clone().to(f.unitOverride);
  //             } catch (e) {
  //
  //             }
  //           }
  //           const formatted = math.format(val, {
  //             precision: 4,
  //             notation: "exponential"
  //           });
  //
  //           const normalizedInput = expr.replace(/\s+/g, '');
  //           const normalizedOutput = formatted.replace(/\s+/g, '');
  //
  //           if (normalizedInput.endsWith(normalizedOutput)) {
  //             results[i].result = "";
  //             return;
  //           }
  //           // val = math.round(val, 10);
  //           results[i].result = mathJsResultToLatex(
  //             math.format(displayVal, { precision: 4, notation: "exponential" })
  //           ).replace(/,/g, ";")
  //             .replace(/\./g, ",\\!")
  //             .replace(/(\d)\s+(?!deg\b)([a-zA-Z]+)/g, "$1\\, $2");
  //           // results[i].result = math.format(val, { precision: 4 })
  //         }
  //
  //         // results[i].result = math.format(val, { precision: 10 });
  //         // results[i].error = null;
  //       } catch(e) {
  //         // console.error(e.message);
  //         results[i].error = e.message;
  //         results[i].result = null;
  //       }
  //     });
  //   }
  //
  //   bridge.receiveResults(JSON.stringify(results));
  // });

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

    let unitOverrideDiv = document.createElement("input");
    unitOverrideDiv.value = f.unitOverride;
    unitOverrideDiv.className = "unit-override hidden";
    unitOverrideDiv.addEventListener("input", () => {
      bridge.updateUnitoverride(f.id, unitOverrideDiv.value);
    })
    if (f.result != null && f.result != "") {
      unitOverrideDiv.className = "unit-override";
    }
    lowerContainer.appendChild(unitOverrideDiv);

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

    // Answer toggle button
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

    // Hiden answer toggle button
    let hideAnswerBtn = document.createElement("button");
    hideAnswerBtn.innerText = "Hide Answer";
    hideAnswerBtn.className = "btn";
    hideAnswerBtn._hideAnswer = f.hideAnswer;
    if (f.isAnswer) {
      hideAnswerBtn.className = "btn active";
    }
    hideAnswerBtn.addEventListener("click", () => {
      // console.log("OMG!");
      hideAnswerBtn._hideAnswer = !hideAnswerBtn._hideAnswer;
      if (hideAnswerBtn._hideAnswer) {
        hideAnswerBtn.className = "btn active";
      } else {
        hideAnswerBtn.className = "btn";
      }
      bridge.toggleHideAnswer(f.id);
    })
    buttonsContainer.appendChild(hideAnswerBtn);

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

