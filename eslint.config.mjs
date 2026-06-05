import js from "@eslint/js";
import globals from "globals";

export default [
  js.configs.recommended,
  {
    languageOptions: {
      ecmaVersion: 2022,
      sourceType: "module",
      globals: {
        ...globals.node,
        ...globals.mocha,
      },
    },
    rules: {
      "no-unused-vars": [
        "warn",
        {
          "argsIgnorePattern": "^_",
          "varsIgnorePattern": "^_",
          "caughtErrorsIgnorePattern": "^_"
        }
      ],
      "no-undef": "error",
    },
  },
  {
    ignores: ["build/*", "node_modules/*", "ACE_TAO/*", "OpenDDS/*", "MPC/*"],
  },
];
