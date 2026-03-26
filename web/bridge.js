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

function latexToMathjs(latex) {
  var fns = ['arcsin','arccos','arctan','asin','acos','atan','sin','cos','tan','sqrt','log','ln','exp'];
  var placeholders = {};

  // strip backslashes from known functions and protect them
  var result = latex;
  fns.forEach(function(fn, i) {
    var ph = '__FN' + i + '__';
    placeholders[ph] = fn;
    result = result.replace(new RegExp('\\\\' + fn, 'g'), ph); // \sin → __FN0__
    result = result.replace(new RegExp(fn, 'g'), ph);          // bare sin → __FN0__
  });

  result = result
    .replace(/\\left\(/g, '(')
    .replace(/\\right\)/g, ')')
    .replace(/\\left\[/g, '[')
    .replace(/\\right\]/g, ']')
    .replace(/\\frac\{([^}]+)\}\{([^}]+)\}/g, '($1)/($2)')
    .replace(/\\cdot/g, '*')
    .replace(/\\pi/g, 'pi')
    .replace(/\\theta/g, 'theta')
    .replace(/\{/g, '(')
    .replace(/\}/g, ')')
    .replace(/([a-zA-Z0-9])([a-zA-Z])/g, '$1*$2')
    .replace(/([0-9])\s*\(/g, '$1*(')
    .replace(/\)\s*\(/g, ')*(');

  // restore
  Object.keys(placeholders).forEach(function(ph) {
    result = result.replace(new RegExp(ph, 'g'), placeholders[ph]);
  });

  return result.trim();
}


// function latexToMathjs(latex) {
//   var result = latex
//     .replace(/\\left\(/g, '(')
//     .replace(/\\right\)/g, ')')
//     .replace(/\\left\[/g, '[')
//     .replace(/\\right\]/g, ']')
//     .replace(/\\frac\{([^}]+)\}\{([^}]+)\}/g, '($1)/($2)')
//     .replace(/\\sqrt\{([^}]+)\}/g, 'sqrt($1)')
//     .replace(/\\cdot/g, '*')
//     .replace(/\\pi/g, 'pi')
//     .replace(/\\theta/g, 'theta')
//     .replace(/\{/g, '(')
//     .replace(/\}/g, ')');
//       // protect known multi-letter names with placeholders before implicit multiply
//   var fns = ['arcsin','arccos','arctan','asin','acos','atan','sin','cos','tan','sqrt','log','ln','exp','dot','norm','point','pi'];
//   var placeholders = {};
//   fns.forEach(function(fn, i) {
//     var ph = '__' + i + '__';
//     placeholders[ph] = fn;
//     result = result.replace(new RegExp(fn, 'g'), ph);
//   });
//
//   // now safe to do implicit multiplication
//   result = result
//     .replace(/([a-zA-Z0-9])([a-zA-Z])/g, '$1*$2')
//     .replace(/([0-9])\s*\(/g, '$1*(')
//     .replace(/\)\s*\(/g, ')*(');
//
//   // restore placeholders
//   Object.keys(placeholders).forEach(function(ph) {
//     result = result.replace(new RegExp(ph, 'g'), placeholders[ph]);
//   });
//
//   return result.trim();
// }

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
    console.error(json);
    renderResults(JSON.parse(json).formulas);
  })

  // AI MADE THIS!!!
  bridge.evaluateTask.connect(function(json) {
    console.error("EVALUATING!!!");
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
          var expr = latexToMathjs(f.latex);
          console.error(expr);
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

          var val = math.evaluate(expr, scope);
          results[i].result = math.format(val, { precision: 6 });
          results[i].error = null;
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
  if (!task) return;
  console.log(task);
  var container = document.getElementById('formulas');
  container.innerHTML = '';
  fields = {};

  task.formulas.forEach(function(f) {
    console.log(f);
    var row = document.createElement("div");
    row.className = "formula-row";

    var mqSpan = document.createElement("span");
    mqSpan.className = "input-field";
    row.appendChild(mqSpan);

    // remove btn
    var removeBtn = document.createElement("button");
    removeBtn.className = "remove-btn"
    removeBtn.textContent = "X";
    removeBtn.addEventListener("click", function() {
      bridge.removeFormula(f.id);
    });
    row.appendChild(removeBtn);

    // Result/error
    var resultDiv = document.createElement("span");
    resultDiv.id = "result-" + f.id;
    resultDiv.className = "result";
    row.appendChild(resultDiv);

    container.appendChild(row);

    var mf = MQ.MathField(mqSpan, {
      autoCommands: 'pi theta sqrt sum',
      charsThatBreakOutOfSupSub: '+-=<>',
      autoSubscriptNumerals: true,
      autoOperatorNames: 'sin cos tan asin acos atan arcsin arccos arctan',
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
          var ids = Object.keys(fields).map(Number);
          var nextId = ids[ids.indexOf(f.id) - 1];
          if (nextId != undefined) {
            fields[nextId].focus();
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

    if (idToFocus == f.id) {
      mf.focus();
      idToFocus = null;
    }
    if (fields[idToFocus])
      fields[idToFocus].focus();
  })
}

function renderResults(results) {
  results.forEach(function(r) {
    let span = document.getElementById("result-" + r.id);
    if (!span) return;
    if (r.error) {
      span.className = "result error";
      span.textContent = r.error;
    } else if (r.result != "") {
      span.className = "result";
      span.textContent = "= " + r.result;
    } else {
      span.className = "result hidden";
      span.textContent = "";
    }
  })
}

