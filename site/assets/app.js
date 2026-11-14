"use strict";
for (const button of document.querySelectorAll("[data-copy]")) {
  button.addEventListener("click", async () => {
    const target = document.getElementById(button.dataset.copy);
    try {
      await navigator.clipboard.writeText(target.textContent);
      button.textContent = "Copied";
      document.getElementById("copy-status").textContent = "Build command copied.";
      setTimeout(() => { button.textContent = "Copy"; }, 2000);
    } catch {
      const selection = window.getSelection();
      const range = document.createRange();
      range.selectNodeContents(target); selection.removeAllRanges(); selection.addRange(range);
      button.textContent = "Select & copy";
      document.getElementById("copy-status").textContent = "Command selected. Use your device’s copy action.";
    }
  });
}
