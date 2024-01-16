export default {
  docsRepositoryBase: "https://github.com/rindag-devs/cplib/tree/main/docs",
  useNextSeoProps() {
    return {
      titleTemplate: "%s - CPLib Docs",
    };
  },
  logo: (
    <div style={{ display: "flex", alignItems: "center", gap: "8px" }}>
      <strong>CPLib</strong> Docs
    </div>
  ),
  project: {
    link: "https://github.com/rindag-devs/cplib",
  },
  search: {
    component: null,
  },
  i18n: [
    { locale: "en", text: "English" },
    { locale: "zh", text: "中文" },
  ],
  footer: {
    text: (
      <div style={{ width: "100%", textAlign: "left" }}>
        CPLib made by{" "}
        <a
          style={{ textDecoration: "underline" }}
          href="https://github.com/rindag-devs"
          target="_blank"
          rel="noopener"
        >
          rindag-devs
        </a>
      </div>
    ),
  },
};
