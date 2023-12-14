const withNextra = require("nextra")({
  theme: "nextra-theme-docs",
  themeConfig: "./theme.config.jsx",
  latex: true,
});

module.exports = withNextra({
  i18n: {
    locales: ["zh", "en"],
    defaultLocale: "en",
    localeDetection: false, // https://github.com/vercel/next.js/issues/55648
  },
});
