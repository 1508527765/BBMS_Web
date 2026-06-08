<template>
  <div class="card" :class="{ 'flx-none': flex }">
    <div class="card-header">
      <h4>{{ title }}</h4>
      <slot name="headerRight"></slot>
    </div>
    <div class="card-body" :class="{ 'flx-none': flex }">
      <slot></slot>
    </div>
  </div>
</template>

<script>
export default {
  name: "cardTemplate",
  props: {
    title: String,
    flex: {
      default: false,
      type: Boolean,
    },
  },
};
</script>
<style scoped>
* {
  margin: 0;
  padding: 0;
  font-size: 13px;
  color: #333;
}

a {
  text-decoration: none;
  color: #666;
}

ul,
li {
  list-style-type: none;
}

img {
  max-width: 100%;
}

.fl {
  float: left;
}

.fr {
  float: right;
}

.p-1 {
  display: inline-block;
  white-space: nowrap;
  text-overflow: ellipsis;
  overflow: hidden;
}

.p-2 {
  display: -webkit-box;
  -webkit-box-orient: vertical;
  -webkit-line-clamp: 2;
  overflow: hidden;
}

.p-3 {
  display: -webkit-box;
  -webkit-box-orient: vertical;
  -webkit-line-clamp: 3;
  overflow: hidden;
}

.p-4 {
  display: -webkit-box;
  -webkit-box-orient: vertical;
  -webkit-line-clamp: 4;
  overflow: hidden;
}

.clear::after {
  content: "";
  display: block;
  width: 100%;
  height: 1px;
  clear: both;
}

.flx-none {
  flex: none !important;
}

.col {
  display: flex;
  flex-wrap: wrap;
  flex-direction: column;
}

.col-9 {
  width: 66.6666666%;
}

.col-3 {
  width: 33.3333333%;
}

.card {
  width: 100%;
  flex: 2;
  display: flex;
  flex-direction: column;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.card-header a {
  color: #fff;
  font-size: 15px;
}

.card-header h4 {
  width: 300px;
  color: #fff;
  font-size: 18px;
  font-weight: normal;
  padding-left: 20px;
  position: relative;
  line-height: 35px;
  background: linear-gradient(to right, #4d4e4d 0%, #4d4e4d 5%, #4d4e4d 100%);
}

.card-header h4::before {
  content: "";
  display: block;
  width: 8px;
  height: 18px;
  background-color: #a8dadc;
  position: absolute;
  left: -4px;
  top: 8px;
}

.card-body {
  padding: 25px 0;
  flex: 2;
}
</style>
